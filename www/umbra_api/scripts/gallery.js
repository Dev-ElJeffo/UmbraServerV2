/**
 * GALLERY.JS - Sistema de Galeria de Arte Conceitual
 * Sistema completo de galeria com filtros, lightbox e lazy loading
 */

class ConceptGallery {
    constructor(options = {}) {
        this.container = options.container || document.getElementById('conceptsGrid');
        this.filtersContainer = options.filters || document.getElementById('conceptsFilters');
        this.galleryDataPath = options.dataPath || 'assets/images/concepts/gallery.json';
        this.basePath = options.basePath || 'assets/images/concepts/';
        
        this.data = null;
        this.currentFilter = 'all';
        this.lightbox = null;
        this.currentImageIndex = 0;
        this.images = [];
        
        this.init();
    }
    
    async init() {
        try {
            await this.loadData();
            this.createFilters();
            this.renderGallery();
            this.setupLightbox();
            this.setupLazyLoading();
        } catch (error) {
            console.error('Erro ao inicializar galeria:', error);
            this.showError('Erro ao carregar galeria de imagens');
        }
    }
    
    async loadData() {
        try {
            const response = await fetch(this.galleryDataPath);
            if (!response.ok) throw new Error('Falha ao carregar dados da galeria');
            this.data = await response.json();
        } catch (error) {
            console.error('Erro ao carregar dados:', error);
            this.data = { categories: {} };
        }
    }
    
    createFilters() {
        if (!this.filtersContainer) return;
        
        this.filtersContainer.innerHTML = '';
        
        // Botão "Todos"
        const allCount = this.getTotalImagesCount();
        const allBtn = this.createFilterButton('all', 'Todos', allCount);
        this.filtersContainer.appendChild(allBtn);
        
        // Botões de categorias
        Object.entries(this.data.categories).forEach(([key, category]) => {
            const count = category.images?.length || 0;
            const btn = this.createFilterButton(key, category.name, count);
            this.filtersContainer.appendChild(btn);
        });
    }
    
    createFilterButton(filter, label, count) {
        const btn = document.createElement('button');
        btn.className = `filter-btn ${filter === this.currentFilter ? 'active' : ''}`;
        btn.dataset.filter = filter;
        btn.innerHTML = `
            ${label}
            ${count > 0 ? `<span class="count">${count}</span>` : ''}
        `;
        
        btn.addEventListener('click', () => {
            this.setFilter(filter);
        });
        
        return btn;
    }
    
    setFilter(filter) {
        this.currentFilter = filter;
        
        // Atualizar botões ativos
        this.filtersContainer.querySelectorAll('.filter-btn').forEach(btn => {
            if (btn.dataset.filter === filter) {
                btn.classList.add('active');
            } else {
                btn.classList.remove('active');
            }
        });
        
        // Filtrar cards
        this.filterCards();
    }
    
    filterCards() {
        const cards = this.container.querySelectorAll('.concept-card');
        
        cards.forEach((card, index) => {
            const category = card.dataset.category;
            
            if (this.currentFilter === 'all' || category === this.currentFilter) {
                card.classList.remove('hidden');
                card.style.animationDelay = `${index * 0.1}s`;
            } else {
                card.classList.add('hidden');
            }
        });
        
        // Atualizar array de imagens visíveis para lightbox
        this.updateVisibleImages();
    }
    
    renderGallery() {
        if (!this.container) return;
        
        this.container.innerHTML = '';
        let hasImages = false;
        
        // Renderizar todas as imagens de todas as categorias
        Object.entries(this.data.categories).forEach(([categoryKey, category]) => {
            if (!category.images || category.images.length === 0) return;
            
            hasImages = true;
            
            category.images.forEach((image, index) => {
                const card = this.createCard(image, categoryKey, category.name);
                this.container.appendChild(card);
            });
        });
        
        if (!hasImages) {
            this.showEmptyState();
        }
        
        // Atualizar imagens visíveis
        this.updateVisibleImages();
    }
    
    createCard(image, category, categoryName) {
        const card = document.createElement('div');
        card.className = 'concept-card scroll-reveal';
        card.dataset.category = category;
        card.dataset.filename = image.filename;
        
        const imagePath = `${this.basePath}${category}/${image.filename}`;
        
        card.innerHTML = `
            <div class="concept-image-wrapper">
                <img class="concept-image" 
                     data-src="${imagePath}" 
                     alt="${image.title || 'Arte Conceitual'}"
                     loading="lazy">
                <span class="category-badge">${categoryName}</span>
                <div class="concept-overlay">
                    <h3 class="concept-title">${image.title || 'Sem título'}</h3>
                    ${image.description ? `<p class="concept-description">${image.description}</p>` : ''}
                    ${image.tags && image.tags.length > 0 ? `
                        <div class="concept-tags">
                            ${image.tags.map(tag => `<span class="concept-tag">${tag}</span>`).join('')}
                        </div>
                    ` : ''}
                </div>
            </div>
        `;
        
        card.addEventListener('click', () => {
            const visibleIndex = this.images.findIndex(img => 
                img.category === category && img.filename === image.filename
            );
            this.openLightbox(visibleIndex >= 0 ? visibleIndex : 0);
        });
        
        return card;
    }
    
    updateVisibleImages() {
        this.images = [];
        
        Object.entries(this.data.categories).forEach(([categoryKey, category]) => {
            if (!category.images) return;
            
            category.images.forEach(image => {
                if (this.currentFilter === 'all' || this.currentFilter === categoryKey) {
                    this.images.push({
                        ...image,
                        category: categoryKey,
                        categoryName: category.name,
                        path: `${this.basePath}${categoryKey}/${image.filename}`
                    });
                }
            });
        });
    }
    
    setupLightbox() {
        // Criar elemento lightbox
        this.lightbox = document.createElement('div');
        this.lightbox.className = 'lightbox';
        this.lightbox.innerHTML = `
            <div class="lightbox-content">
                <img class="lightbox-image" src="" alt="">
                <div class="lightbox-info">
                    <h3 class="lightbox-title"></h3>
                    <p class="lightbox-description"></p>
                </div>
            </div>
            <button class="lightbox-close" aria-label="Fechar">
                <i class="fas fa-times"></i>
            </button>
            <button class="lightbox-nav prev" aria-label="Anterior">
                <i class="fas fa-chevron-left"></i>
            </button>
            <button class="lightbox-nav next" aria-label="Próxima">
                <i class="fas fa-chevron-right"></i>
            </button>
            <div class="lightbox-counter">
                <span class="current">1</span> / <span class="total">1</span>
            </div>
        `;
        
        document.body.appendChild(this.lightbox);
        
        // Event listeners
        this.lightbox.querySelector('.lightbox-close').addEventListener('click', () => {
            this.closeLightbox();
        });
        
        this.lightbox.querySelector('.prev').addEventListener('click', () => {
            this.navigateLightbox(-1);
        });
        
        this.lightbox.querySelector('.next').addEventListener('click', () => {
            this.navigateLightbox(1);
        });
        
        // Fechar ao clicar fora
        this.lightbox.addEventListener('click', (e) => {
            if (e.target === this.lightbox) {
                this.closeLightbox();
            }
        });
        
        // Keyboard navigation
        document.addEventListener('keydown', (e) => {
            if (!this.lightbox.classList.contains('active')) return;
            
            switch(e.key) {
                case 'Escape':
                    this.closeLightbox();
                    break;
                case 'ArrowLeft':
                    this.navigateLightbox(-1);
                    break;
                case 'ArrowRight':
                    this.navigateLightbox(1);
                    break;
            }
        });
    }
    
    openLightbox(index) {
        if (!this.images || this.images.length === 0) return;
        
        this.currentImageIndex = index;
        this.updateLightboxContent();
        this.lightbox.classList.add('active');
        document.body.style.overflow = 'hidden';
    }
    
    closeLightbox() {
        this.lightbox.classList.remove('active');
        document.body.style.overflow = '';
    }
    
    navigateLightbox(direction) {
        this.currentImageIndex += direction;
        
        if (this.currentImageIndex < 0) {
            this.currentImageIndex = this.images.length - 1;
        } else if (this.currentImageIndex >= this.images.length) {
            this.currentImageIndex = 0;
        }
        
        this.updateLightboxContent();
    }
    
    updateLightboxContent() {
        if (!this.images || this.images.length === 0) return;
        
        const image = this.images[this.currentImageIndex];
        const img = this.lightbox.querySelector('.lightbox-image');
        const title = this.lightbox.querySelector('.lightbox-title');
        const description = this.lightbox.querySelector('.lightbox-description');
        const counter = this.lightbox.querySelector('.lightbox-counter');
        
        img.src = image.path;
        img.alt = image.title || 'Arte Conceitual';
        title.textContent = image.title || 'Sem título';
        description.textContent = image.description || '';
        
        counter.querySelector('.current').textContent = this.currentImageIndex + 1;
        counter.querySelector('.total').textContent = this.images.length;
        
        // Atualizar estado dos botões de navegação
        const prevBtn = this.lightbox.querySelector('.prev');
        const nextBtn = this.lightbox.querySelector('.next');
        
        prevBtn.disabled = this.images.length <= 1;
        nextBtn.disabled = this.images.length <= 1;
    }
    
    setupLazyLoading() {
        const imageObserver = new IntersectionObserver((entries, observer) => {
            entries.forEach(entry => {
                if (entry.isIntersecting) {
                    const img = entry.target;
                    const src = img.dataset.src;
                    
                    if (src) {
                        img.src = src;
                        img.removeAttribute('data-src');
                        img.classList.add('loaded');
                    }
                    
                    observer.unobserve(img);
                }
            });
        }, {
            rootMargin: '50px'
        });
        
        // Observar todas as imagens com data-src
        const images = this.container.querySelectorAll('img[data-src]');
        images.forEach(img => imageObserver.observe(img));
    }
    
    getTotalImagesCount() {
        let total = 0;
        Object.values(this.data.categories).forEach(category => {
            total += category.images?.length || 0;
        });
        return total;
    }
    
    showEmptyState() {
        this.container.innerHTML = `
            <div class="concepts-empty">
                <i class="fas fa-images"></i>
                <h3>Nenhuma imagem disponível</h3>
                <p>As artes conceituais de Umbra Eternum serão adicionadas em breve. Adicione suas próprias imagens editando o arquivo gallery.json!</p>
            </div>
        `;
    }
    
    showError(message) {
        if (!this.container) return;
        
        this.container.innerHTML = `
            <div class="concepts-empty">
                <i class="fas fa-exclamation-triangle"></i>
                <h3>Erro ao Carregar Galeria</h3>
                <p>${message}</p>
            </div>
        `;
    }
}

// Inicializar quando a página carregar
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => {
        if (document.getElementById('conceptsGrid')) {
            window.conceptGallery = new ConceptGallery();
        }
    });
} else {
    if (document.getElementById('conceptsGrid')) {
        window.conceptGallery = new ConceptGallery();
    }
}
