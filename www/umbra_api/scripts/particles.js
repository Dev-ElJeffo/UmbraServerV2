/**
 * UmbraEternum - Sistema de Partículas
 * Efeitos visuais sutis para atmosfera dark fantasy
 */

class ParticleSystem {
    constructor(container, options = {}) {
        this.container = container;
        this.options = {
            count: options.count || 30,
            color: options.color || 'var(--color-gold-bright)',
            minSize: options.minSize || 2,
            maxSize: options.maxSize || 4,
            speed: options.speed || 5000,
            spread: options.spread || 100
        };
        
        this.particles = [];
        this.isRunning = false;
    }

    init() {
        this.container.style.position = 'relative';
        this.container.style.overflow = 'hidden';
        
        for (let i = 0; i < this.options.count; i++) {
            this.createParticle();
        }
        
        return this;
    }

    createParticle() {
        const particle = document.createElement('div');
        particle.className = 'particle';
        
        const size = Math.random() * (this.options.maxSize - this.options.minSize) + this.options.minSize;
        const startX = Math.random() * 100;
        const startY = Math.random() * 100;
        const endX = (Math.random() - 0.5) * this.options.spread;
        const endY = -(Math.random() * 100 + 50);
        const duration = this.options.speed + Math.random() * 2000;
        const delay = Math.random() * 5000;
        
        particle.style.cssText = `
            width: ${size}px;
            height: ${size}px;
            left: ${startX}%;
            top: ${startY}%;
            background: ${this.options.color};
            animation: floatParticle ${duration}ms ease-in-out ${delay}ms infinite;
            --particle-x: ${endX}px;
            --particle-y: ${endY}px;
        `;
        
        this.container.appendChild(particle);
        this.particles.push(particle);
    }

    start() {
        this.isRunning = true;
        return this;
    }

    stop() {
        this.isRunning = false;
        this.particles.forEach(p => p.remove());
        this.particles = [];
        return this;
    }
}

// ========================================
// FOG EFFECT
// ========================================

class FogEffect {
    constructor(container) {
        this.container = container;
        this.fog = null;
    }

    init() {
        this.fog = document.createElement('div');
        this.fog.className = 'fog-overlay';
        this.container.style.position = 'relative';
        this.container.appendChild(this.fog);
        return this;
    }

    remove() {
        if (this.fog) {
            this.fog.remove();
            this.fog = null;
        }
        return this;
    }
}

// ========================================
// GLOW EFFECT ON ELEMENTS
// ========================================

class GlowEffect {
    static apply(element, type = 'gold') {
        const glowClass = {
            gold: 'animate-glowPulse',
            purple: 'animate-mysticalGlow',
            red: 'animate-bloodGlow'
        };
        
        element.classList.add(glowClass[type] || glowClass.gold);
    }

    static remove(element) {
        element.classList.remove('animate-glowPulse', 'animate-mysticalGlow', 'animate-bloodGlow');
    }
}

// ========================================
// BACKGROUND ANIMATION
// ========================================

class BackgroundAnimation {
    constructor(container) {
        this.container = container;
        this.canvas = null;
        this.ctx = null;
        this.particles = [];
        this.animationId = null;
    }

    init(particleCount = 50) {
        this.canvas = document.createElement('canvas');
        this.canvas.style.position = 'absolute';
        this.canvas.style.top = '0';
        this.canvas.style.left = '0';
        this.canvas.style.width = '100%';
        this.canvas.style.height = '100%';
        this.canvas.style.pointerEvents = 'none';
        this.canvas.style.opacity = '0.3';
        
        this.container.style.position = 'relative';
        this.container.appendChild(this.canvas);
        
        this.ctx = this.canvas.getContext('2d');
        this.resize();
        
        window.addEventListener('resize', () => this.resize());
        
        // Criar partículas
        for (let i = 0; i < particleCount; i++) {
            this.particles.push({
                x: Math.random() * this.canvas.width,
                y: Math.random() * this.canvas.height,
                radius: Math.random() * 2 + 1,
                vx: (Math.random() - 0.5) * 0.5,
                vy: (Math.random() - 0.5) * 0.5,
                alpha: Math.random() * 0.5 + 0.25
            });
        }
        
        this.animate();
        return this;
    }

    resize() {
        const rect = this.container.getBoundingClientRect();
        this.canvas.width = rect.width;
        this.canvas.height = rect.height;
    }

    animate() {
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
        
        this.particles.forEach(particle => {
            // Atualizar posição
            particle.x += particle.vx;
            particle.y += particle.vy;
            
            // Wrap around
            if (particle.x < 0) particle.x = this.canvas.width;
            if (particle.x > this.canvas.width) particle.x = 0;
            if (particle.y < 0) particle.y = this.canvas.height;
            if (particle.y > this.canvas.height) particle.y = 0;
            
            // Desenhar
            this.ctx.beginPath();
            this.ctx.arc(particle.x, particle.y, particle.radius, 0, Math.PI * 2);
            this.ctx.fillStyle = `rgba(212, 175, 55, ${particle.alpha})`;
            this.ctx.fill();
        });
        
        this.animationId = requestAnimationFrame(() => this.animate());
    }

    stop() {
        if (this.animationId) {
            cancelAnimationFrame(this.animationId);
            this.animationId = null;
        }
        if (this.canvas) {
            this.canvas.remove();
            this.canvas = null;
        }
        return this;
    }
}

// ========================================
// TEXT GLITCH EFFECT
// ========================================

class GlitchEffect {
    static apply(element, duration = 3000) {
        const originalText = element.textContent;
        const chars = '!<>-_\\/[]{}—=+*^?#________';
        let iterations = 0;
        const maxIterations = 10;
        
        const interval = setInterval(() => {
            element.textContent = originalText
                .split('')
                .map((char, index) => {
                    if (index < iterations) {
                        return originalText[index];
                    }
                    return chars[Math.floor(Math.random() * chars.length)];
                })
                .join('');
            
            iterations += 1 / 3;
            
            if (iterations >= originalText.length) {
                clearInterval(interval);
                element.textContent = originalText;
            }
        }, duration / maxIterations);
    }
}

// ========================================
// TYPEWRITER EFFECT
// ========================================

class TypewriterEffect {
    static type(element, text, speed = 50) {
        return new Promise((resolve) => {
            element.textContent = '';
            let i = 0;
            
            const interval = setInterval(() => {
                if (i < text.length) {
                    element.textContent += text.charAt(i);
                    i++;
                } else {
                    clearInterval(interval);
                    resolve();
                }
            }, speed);
        });
    }
}

// ========================================
// PARALLAX EFFECT
// ========================================

class ParallaxEffect {
    static init() {
        const parallaxElements = document.querySelectorAll('[data-parallax]');
        
        if (parallaxElements.length === 0) return;
        
        const handleScroll = Utils.throttle(() => {
            const scrolled = window.pageYOffset;
            
            parallaxElements.forEach(element => {
                const speed = parseFloat(element.getAttribute('data-parallax')) || 0.5;
                const yPos = -(scrolled * speed);
                element.style.transform = `translateY(${yPos}px)`;
            });
        }, 10);
        
        window.addEventListener('scroll', handleScroll);
        handleScroll();
    }
}

// ========================================
// SCREEN SHAKE
// ========================================

class ScreenShake {
    static shake(element = document.body, intensity = 5, duration = 500) {
        const originalTransform = element.style.transform;
        const start = Date.now();
        
        const animate = () => {
            const elapsed = Date.now() - start;
            const progress = Math.min(elapsed / duration, 1);
            const currentIntensity = intensity * (1 - progress);
            
            if (progress < 1) {
                const x = (Math.random() - 0.5) * currentIntensity * 2;
                const y = (Math.random() - 0.5) * currentIntensity * 2;
                element.style.transform = `translate(${x}px, ${y}px)`;
                requestAnimationFrame(animate);
            } else {
                element.style.transform = originalTransform;
            }
        };
        
        animate();
    }
}

// ========================================
// INICIALIZAÇÃO GLOBAL
// ========================================

document.addEventListener('DOMContentLoaded', () => {
    // Inicializar parallax se houver elementos
    ParallaxEffect.init();
    
    // Adicionar efeitos hover em cards
    document.querySelectorAll('.card-medieval, .class-card').forEach(card => {
        card.addEventListener('mouseenter', () => {
            card.style.transform = 'translateY(-4px)';
        });
        
        card.addEventListener('mouseleave', () => {
            card.style.transform = '';
        });
    });
});

// Exportar para global
window.ParticleSystem = ParticleSystem;
window.FogEffect = FogEffect;
window.GlowEffect = GlowEffect;
window.BackgroundAnimation = BackgroundAnimation;
window.GlitchEffect = GlitchEffect;
window.TypewriterEffect = TypewriterEffect;
window.ParallaxEffect = ParallaxEffect;
window.ScreenShake = ScreenShake;
