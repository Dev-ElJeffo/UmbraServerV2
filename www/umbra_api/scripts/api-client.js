/**
 * UmbraEternum - API Client
 * Wrapper para comunicação com as APIs PHP do servidor
 */

class UmbraAPI {
    constructor() {
        this.baseURL = window.location.origin + '/umbra_api/api';
        this.token = this.getToken();
    }

    /**
     * Obter token do sessionStorage
     */
    getToken() {
        return sessionStorage.getItem('auth_token') || '';
    }

    /**
     * Salvar token no sessionStorage
     */
    setToken(token) {
        this.token = token;
        sessionStorage.setItem('auth_token', token);
    }

    /**
     * Remover token (logout)
     */
    clearToken() {
        this.token = '';
        sessionStorage.removeItem('auth_token');
        sessionStorage.removeItem('user_data');
        sessionStorage.removeItem('players');
    }

    /**
     * Request genérico
     */
    async request(endpoint, options = {}) {
        const {
            method = 'POST',
            body = null,
            headers = {},
            requiresAuth = false
        } = options;

        const config = {
            method,
            headers: {
                'Content-Type': 'application/json',
                ...headers
            }
        };

        // Adicionar body se for POST
        if (body && method !== 'GET') {
            config.body = JSON.stringify(body);
        }

        try {
            const response = await fetch(`${this.baseURL}/${endpoint}`, config);
            const data = await response.json();

            if (!response.ok) {
                throw new Error(data.message || 'Erro na requisição');
            }

            return data;
        } catch (error) {
            console.error(`API Error [${endpoint}]:`, error);
            throw error;
        }
    }

    // ========================================
    // AUTENTICAÇÃO
    // ========================================

    /**
     * Login
     */
    async login(username, password) {
        const response = await this.request('login.php', {
            body: { username, password }
        });

        if (response.success && response.token) {
            this.setToken(response.token);
            sessionStorage.setItem('user_data', JSON.stringify(response.account));
            sessionStorage.setItem('players', JSON.stringify(response.players));
        }

        return response;
    }

    /**
     * Registro
     */
    async register(username, email, password) {
        return await this.request('register.php', {
            body: { username, email, password }
        });
    }

    /**
     * Logout
     */
    logout() {
        this.clearToken();
        window.location.href = 'login.html';
    }

    /**
     * Verificar se está autenticado
     */
    isAuthenticated() {
        return !!this.token;
    }

    /**
     * Obter dados do usuário
     */
    getUserData() {
        const data = sessionStorage.getItem('user_data');
        return data ? JSON.parse(data) : null;
    }

    /**
     * Obter lista de personagens
     */
    getPlayersList() {
        const data = sessionStorage.getItem('players');
        return data ? JSON.parse(data) : [];
    }

    // ========================================
    // PERSONAGENS
    // ========================================

    /**
     * Obter classes disponíveis
     */
    async getClasses() {
        return await this.request('character/get_classes.php', {
            method: 'GET'
        });
    }

    /**
     * Criar personagem
     */
    async createCharacter(characterName, classId) {
        return await this.request('character/create_character.php', {
            body: {
                token: this.token,
                character_name: characterName,
                class_id: classId
            }
        });
    }

    /**
     * Selecionar personagem
     */
    async selectCharacter(playerId) {
        return await this.request('character/select_character.php', {
            body: {
                token: this.token,
                player_id: playerId
            }
        });
    }

    /**
     * Listar personagens
     */
    async listCharacters() {
        return await this.request('character/list_characters.php', {
            body: { token: this.token }
        });
    }

    /**
     * Deletar personagem
     */
    async deleteCharacter(playerId) {
        return await this.request('character/delete_character.php', {
            body: {
                token: this.token,
                player_id: playerId
            }
        });
    }

    /**
     * Obter informações do personagem
     */
    async getCharacterInfo(playerId) {
        return await this.request('character/get_character_info.php', {
            body: {
                token: this.token,
                player_id: playerId
            }
        });
    }

    // ========================================
    // INVENTÁRIO
    // ========================================

    /**
     * Obter inventário
     */
    async getInventory() {
        return await this.request('inventory/get_inventory.php', {
            body: { token: this.token }
        });
    }

    /**
     * Adicionar item
     */
    async addItem(itemTemplateId, quantity = 1) {
        return await this.request('inventory/add_item.php', {
            body: {
                token: this.token,
                item_template_id: itemTemplateId,
                quantity
            }
        });
    }

    /**
     * Remover item
     */
    async removeItem(itemId, quantity = 1) {
        return await this.request('inventory/remove_item.php', {
            body: {
                token: this.token,
                item_id: itemId,
                quantity
            }
        });
    }

    /**
     * Equipar item
     */
    async equipItem(itemId) {
        return await this.request('inventory/equip_item.php', {
            body: {
                token: this.token,
                item_id: itemId
            }
        });
    }

    /**
     * Mover item
     */
    async moveItem(itemId, targetSlot) {
        return await this.request('inventory/move_item.php', {
            body: {
                token: this.token,
                item_id: itemId,
                target_slot: targetSlot
            }
        });
    }

    // ========================================
    // SKILLS
    // ========================================

    /**
     * Obter skills do jogador
     */
    async getPlayerSkills() {
        return await this.request('skills/get_player_skills.php', {
            body: { token: this.token }
        });
    }

    /**
     * Obter skills disponíveis
     */
    async getAvailableSkills() {
        return await this.request('skills/get_available_skills.php', {
            body: { token: this.token }
        });
    }

    /**
     * Aprender skill
     */
    async learnSkill(skillId) {
        return await this.request('skills/learn_skill.php', {
            body: {
                token: this.token,
                skill_id: skillId
            }
        });
    }

    /**
     * Usar skill
     */
    async useSkill(skillId, targetId = null) {
        return await this.request('skills/use_skill.php', {
            body: {
                token: this.token,
                skill_id: skillId,
                target_id: targetId
            }
        });
    }

    /**
     * Obter skillbar
     */
    async getSkillbar() {
        return await this.request('skills/get_skillbar.php', {
            body: { token: this.token }
        });
    }

    /**
     * Definir skillbar
     */
    async setSkillbar(skillbarData) {
        return await this.request('skills/set_skillbar.php', {
            body: {
                token: this.token,
                ...skillbarData
            }
        });
    }

    // ========================================
    // SOCIAL (GUILD)
    // ========================================

    /**
     * Obter estado da guild
     */
    async getGuildState() {
        return await this.request('social/get_guild_state.php', {
            body: { token: this.token }
        });
    }

    /**
     * Criar guild
     */
    async createGuild(guildName) {
        return await this.request('social/create_guild.php', {
            body: {
                token: this.token,
                guild_name: guildName
            }
        });
    }

    /**
     * Convidar membro para guild
     */
    async inviteGuildMember(targetPlayerId) {
        return await this.request('social/invite_guild_member.php', {
            body: {
                token: this.token,
                target_player_id: targetPlayerId
            }
        });
    }

    // ========================================
    // LOJA/AUCTION
    // ========================================

    /**
     * Listar itens do leilão
     */
    async listAuctionListings(filters = {}) {
        return await this.request('auction/list_auction_listings.php', {
            body: {
                token: this.token,
                ...filters
            }
        });
    }

    /**
     * Criar listing no leilão
     */
    async createAuctionListing(itemId, price, duration) {
        return await this.request('auction/create_auction_listing.php', {
            body: {
                token: this.token,
                item_id: itemId,
                price,
                duration
            }
        });
    }

    /**
     * Comprar item do leilão
     */
    async purchaseAuctionListing(listingId) {
        return await this.request('auction/purchase_auction_listing.php', {
            body: {
                token: this.token,
                listing_id: listingId
            }
        });
    }

    // ========================================
    // STORAGE
    // ========================================

    /**
     * Obter storage
     */
    async getStorage() {
        return await this.request('storage/get_storage.php', {
            body: { token: this.token }
        });
    }

    /**
     * Mover para storage
     */
    async moveToStorage(itemId, quantity = 1) {
        return await this.request('storage/move_to_storage.php', {
            body: {
                token: this.token,
                item_id: itemId,
                quantity
            }
        });
    }

    /**
     * Mover do storage
     */
    async moveFromStorage(itemId, quantity = 1) {
        return await this.request('storage/move_from_storage.php', {
            body: {
                token: this.token,
                item_id: itemId,
                quantity
            }
        });
    }

    // ========================================
    // GOLD
    // ========================================

    /**
     * Obter gold
     */
    async getGold() {
        return await this.request('gold/get_gold.php', {
            body: { token: this.token }
        });
    }

    /**
     * Depositar gold
     */
    async depositGold(amount) {
        return await this.request('gold/deposit_gold.php', {
            body: {
                token: this.token,
                amount
            }
        });
    }

    /**
     * Sacar gold
     */
    async withdrawGold(amount) {
        return await this.request('gold/withdraw_gold.php', {
            body: {
                token: this.token,
                amount
            }
        });
    }

    // ========================================
    // ADMIN
    // ========================================

    /**
     * Criar item (admin)
     */
    async adminCreateItem(itemData) {
        return await this.request('admin/create_item.php', {
            body: {
                token: this.token,
                ...itemData
            }
        });
    }

    /**
     * Listar itens (admin)
     */
    async adminListItems() {
        return await this.request('admin/list_items.php', {
            body: { token: this.token }
        });
    }

    /**
     * Deletar item (admin)
     */
    async adminDeleteItem(itemId) {
        return await this.request('admin/delete_item.php', {
            body: {
                token: this.token,
                item_id: itemId
            }
        });
    }

    /**
     * Status do servidor (admin)
     */
    async adminServerStatus() {
        return await this.request('admin/server_status.php', {
            body: { token: this.token }
        });
    }
}

// Instância global
window.umbraAPI = new UmbraAPI();
