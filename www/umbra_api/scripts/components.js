/**
 * UmbraEternum - Componentes JavaScript Reutilizáveis
 * Modal, Tooltip, Toast, etc.
 */

// ========================================
// MODAL SYSTEM
// ========================================

class Modal {
    constructor(options = {}) {
        this.options = {
            title: options.title || 'Modal',
            content: options.content || '',
            footer: options.footer || null,
            onClose: options.onClose || null,
            closeOnBackdrop: options.closeOnBackdrop !== false
        };
        
        this.element = null;
        this.backdrop = null;
        this.isOpen = false;
    }

    create() {
        // Criar backdrop
        this.backdrop = document.createElement('div');
        this.backdrop.className = 'modal-backdrop';
        
        // Criar modal
        const modal = document.createElement('div');
        modal.className = 'modal';
        
        // Header
        const header = document.createElement('div');
        header.className = 'modal-header';
        header.innerHTML = `
            <h3 class="modal-title">${this.options.title}</h3>
            <button class="modal-close" aria-label="Fechar">
                <i class="fas fa-times"></i>
            </button>
        `;
        
        // Body
        const body = document.createElement('div');
        body.className = 'modal-body';
        
        if (typeof this.options.content === 'string') {
            body.innerHTML = this.options.content;
        } else if (this.options.content instanceof HTMLElement) {
            body.appendChild(this.options.content);
        }
        
        // Footer
        if (this.options.footer) {
            const footer = document.createElement('div');
            footer.className = 'modal-footer';
            
            if (typeof this.options.footer === 'string') {
                footer.innerHTML = this.options.footer;
            } else if (this.options.footer instanceof HTMLElement) {
                footer.appendChild(this.options.footer);
            }
            
            modal.appendChild(header);
            modal.appendChild(body);
            modal.appendChild(footer);
        } else {
            modal.appendChild(header);
            modal.appendChild(body);
        }
        
        this.backdrop.appendChild(modal);
        this.element = modal;
        
        // Event listeners
        const closeBtn = header.querySelector('.modal-close');
        closeBtn.addEventListener('click', () => this.close());
        
        if (this.options.closeOnBackdrop) {
            this.backdrop.addEventListener('click', (e) => {
                if (e.target === this.backdrop) {
                    this.close();
                }
            });
        }
        
        return this;
    }

    open() {
        if (!this.backdrop) {
            this.create();
        }
        
        document.body.appendChild(this.backdrop);
        
        // Trigger reflow para animação
        this.backdrop.offsetHeight;
        
        this.backdrop.classList.add('active');
        this.isOpen = true;
        
        // Prevenir scroll do body
        document.body.style.overflow = 'hidden';
        
        return this;
    }

    close() {
        if (!this.isOpen) return;
        
        this.backdrop.classList.remove('active');
        
        setTimeout(() => {
            if (this.backdrop && this.backdrop.parentNode) {
                this.backdrop.parentNode.removeChild(this.backdrop);
            }
            document.body.style.overflow = '';
            this.isOpen = false;
            
            if (this.options.onClose) {
                this.options.onClose();
            }
        }, 300);
        
        return this;
    }

    setContent(content) {
        const body = this.element.querySelector('.modal-body');
        if (typeof content === 'string') {
            body.innerHTML = content;
        } else if (content instanceof HTMLElement) {
            body.innerHTML = '';
            body.appendChild(content);
        }
        return this;
    }

    setTitle(title) {
        const titleEl = this.element.querySelector('.modal-title');
        titleEl.textContent = title;
        return this;
    }
}

// ========================================
// TOAST SYSTEM
// ========================================

class Toast {
    static container = null;
    
    static init() {
        if (!Toast.container) {
            Toast.container = document.createElement('div');
            Toast.container.className = 'toast-container';
            document.body.appendChild(Toast.container);
        }
    }
    
    static show(message, type = 'info', duration = 5000) {
        Toast.init();
        
        const toast = document.createElement('div');
        toast.className = `toast ${type}`;
        
        const icons = {
            success: 'fa-check-circle',
            error: 'fa-exclamation-circle',
            warning: 'fa-exclamation-triangle',
            info: 'fa-info-circle'
        };
        
        toast.innerHTML = `
            <div class="toast-icon">
                <i class="fas ${icons[type] || icons.info}"></i>
            </div>
            <div class="toast-content">
                <div class="toast-message">${message}</div>
            </div>
            <button class="toast-close">
                <i class="fas fa-times"></i>
            </button>
        `;
        
        const closeBtn = toast.querySelector('.toast-close');
        closeBtn.addEventListener('click', () => {
            toast.style.animation = 'slideInRight 0.3s ease reverse';
            setTimeout(() => toast.remove(), 300);
        });
        
        Toast.container.appendChild(toast);
        
        if (duration > 0) {
            setTimeout(() => {
                if (toast.parentNode) {
                    toast.style.animation = 'slideInRight 0.3s ease reverse';
                    setTimeout(() => toast.remove(), 300);
                }
            }, duration);
        }
        
        return toast;
    }
    
    static success(message, duration) {
        return Toast.show(message, 'success', duration);
    }
    
    static error(message, duration) {
        return Toast.show(message, 'error', duration);
    }
    
    static warning(message, duration) {
        return Toast.show(message, 'warning', duration);
    }
    
    static info(message, duration) {
        return Toast.show(message, 'info', duration);
    }
}

// ========================================
// TOOLTIP SYSTEM
// ========================================

class Tooltip {
    static activeTooltip = null;
    
    static init() {
        document.addEventListener('mouseover', (e) => {
            const target = e.target.closest('[data-tooltip]');
            if (target) {
                Tooltip.show(target);
            }
        });
        
        document.addEventListener('mouseout', (e) => {
            const target = e.target.closest('[data-tooltip]');
            if (target) {
                Tooltip.hide();
            }
        });
    }
    
    static show(element) {
        Tooltip.hide();
        
        const text = element.getAttribute('data-tooltip');
        if (!text) return;
        
        const tooltip = document.createElement('div');
        tooltip.className = 'tooltip';
        tooltip.textContent = text;
        
        document.body.appendChild(tooltip);
        
        const rect = element.getBoundingClientRect();
        const tooltipRect = tooltip.getBoundingClientRect();
        
        tooltip.style.left = `${rect.left + (rect.width / 2) - (tooltipRect.width / 2)}px`;
        tooltip.style.top = `${rect.top - tooltipRect.height - 10}px`;
        
        setTimeout(() => tooltip.classList.add('active'), 10);
        
        Tooltip.activeTooltip = tooltip;
    }
    
    static hide() {
        if (Tooltip.activeTooltip) {
            Tooltip.activeTooltip.remove();
            Tooltip.activeTooltip = null;
        }
    }
}

// ========================================
// LOADING OVERLAY
// ========================================

class Loading {
    static overlay = null;
    
    static show(text = 'Carregando...') {
        if (Loading.overlay) return;
        
        Loading.overlay = document.createElement('div');
        Loading.overlay.className = 'loading-overlay';
        Loading.overlay.innerHTML = `
            <div>
                <div class="loading-spinner"></div>
                <div class="loading-text">${text}</div>
            </div>
        `;
        
        document.body.appendChild(Loading.overlay);
        document.body.style.overflow = 'hidden';
    }
    
    static hide() {
        if (Loading.overlay) {
            Loading.overlay.remove();
            Loading.overlay = null;
            document.body.style.overflow = '';
        }
    }
}

// ========================================
// CONFIRM DIALOG
// ========================================

class Confirm {
    static async show(message, title = 'Confirmação') {
        return new Promise((resolve) => {
            const footer = document.createElement('div');
            footer.style.display = 'flex';
            footer.style.gap = 'var(--spacing-md)';
            footer.style.justifyContent = 'flex-end';
            
            const btnCancel = document.createElement('button');
            btnCancel.className = 'btn btn-secondary';
            btnCancel.textContent = 'Cancelar';
            
            const btnConfirm = document.createElement('button');
            btnConfirm.className = 'btn btn-primary';
            btnConfirm.textContent = 'Confirmar';
            
            footer.appendChild(btnCancel);
            footer.appendChild(btnConfirm);
            
            const modal = new Modal({
                title,
                content: `<p style="font-size: 1.1rem; line-height: 1.6;">${message}</p>`,
                footer,
                closeOnBackdrop: false
            });
            
            btnCancel.addEventListener('click', () => {
                modal.close();
                resolve(false);
            });
            
            btnConfirm.addEventListener('click', () => {
                modal.close();
                resolve(true);
            });
            
            modal.open();
        });
    }
}

// ========================================
// SCROLL REVEAL
// ========================================

class ScrollReveal {
    static observer = null;
    
    static init() {
        if (ScrollReveal.observer) return;
        
        const options = {
            root: null,
            rootMargin: '0px',
            threshold: 0.1
        };
        
        ScrollReveal.observer = new IntersectionObserver((entries) => {
            entries.forEach(entry => {
                if (entry.isIntersecting) {
                    entry.target.classList.add('revealed');
                    ScrollReveal.observer.unobserve(entry.target);
                }
            });
        }, options);
        
        document.querySelectorAll('.scroll-reveal, .scroll-reveal-left, .scroll-reveal-right, .scroll-reveal-scale')
            .forEach(el => ScrollReveal.observer.observe(el));
    }
    
    static observe(element) {
        if (!ScrollReveal.observer) {
            ScrollReveal.init();
        }
        ScrollReveal.observer.observe(element);
    }
}

// ========================================
// UTILITY FUNCTIONS
// ========================================

const Utils = {
    /**
     * Formatar números com separador de milhares
     */
    formatNumber(num) {
        return num.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ".");
    },
    
    /**
     * Formatar gold
     */
    formatGold(amount) {
        return `<i class="fas fa-coins" style="color: var(--color-gold-bright)"></i> ${Utils.formatNumber(amount)}`;
    },
    
    /**
     * Obter cor por raridade
     */
    getRarityColor(rarity) {
        const colors = {
            common: 'var(--rarity-common)',
            uncommon: 'var(--rarity-uncommon)',
            rare: 'var(--rarity-rare)',
            epic: 'var(--rarity-epic)',
            legendary: 'var(--rarity-legendary)'
        };
        return colors[rarity] || colors.common;
    },
    
    /**
     * Debounce function
     */
    debounce(func, wait) {
        let timeout;
        return function executedFunction(...args) {
            const later = () => {
                clearTimeout(timeout);
                func(...args);
            };
            clearTimeout(timeout);
            timeout = setTimeout(later, wait);
        };
    },
    
    /**
     * Throttle function
     */
    throttle(func, limit) {
        let inThrottle;
        return function(...args) {
            if (!inThrottle) {
                func.apply(this, args);
                inThrottle = true;
                setTimeout(() => inThrottle = false, limit);
            }
        };
    },
    
    /**
     * Validar email
     */
    validateEmail(email) {
        const re = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
        return re.test(email);
    },
    
    /**
     * Sanitizar HTML
     */
    escapeHTML(str) {
        const div = document.createElement('div');
        div.textContent = str;
        return div.innerHTML;
    },
    
    /**
     * Copiar para clipboard
     */
    async copyToClipboard(text) {
        try {
            await navigator.clipboard.writeText(text);
            Toast.success('Copiado para área de transferência!');
            return true;
        } catch (err) {
            Toast.error('Erro ao copiar');
            return false;
        }
    }
};

// ========================================
// INICIALIZAÇÃO
// ========================================

document.addEventListener('DOMContentLoaded', () => {
    // Inicializar sistemas
    Tooltip.init();
    ScrollReveal.init();
    
    // Prevenir submit de forms sem handler
    document.querySelectorAll('form').forEach(form => {
        if (!form.hasAttribute('data-no-prevent')) {
            form.addEventListener('submit', (e) => {
                if (!form.hasAttribute('data-handled')) {
                    e.preventDefault();
                }
            });
        }
    });
});

// Exportar para global
window.Modal = Modal;
window.Toast = Toast;
window.Tooltip = Tooltip;
window.Loading = Loading;
window.Confirm = Confirm;
window.ScrollReveal = ScrollReveal;
window.Utils = Utils;
