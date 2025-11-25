#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    printf("=== Test de Protección de Lectura ===\n\n");
    
    char *addr = sbrk(0);      // Dirección actual del heap
    sbrk(4096);                // Reservar una página
    printf("✓ Paso 1: Página reservada en dirección %p\n", addr);
    
    addr[0] = 'Z';             // Escribir valor inicial
    printf("✓ Paso 2: Escritura inicial exitosa (valor='Z')\n");

    // Proteger contra lectura
    printf("DEBUG: Llamando mrdprotect(addr=%p, len=1)\n", addr);
    printf("DEBUG: addr %% PGSIZE = %ld (debe ser 0)\n", (uint64)addr % 4096);
    int result = mrdprotect(addr, 1);
    printf("DEBUG: mrdprotect retornó %d\n", result);
    if (result < 0) {
        printf("[ERROR] mrdprotect falló (retornó -1)\n");
        exit(1);
    }
    printf("✓ Paso 3: Protección aplicada exitosamente (mrdprotect)\n");

    printf("\n[ADVERTENCIA] Paso 4: Intentando acceder a página protegida...\n");
    printf("              ESPERADO: El proceso debe terminar con page fault\n\n");
    
    // Intento de acceso (lectura o escritura) debería provocar fallo
    char c = addr[0];
    printf("[ERROR] Acceso exitoso (valor='%c') - LA PROTECCIÓN NO FUNCIONÓ\n", c);
    exit(1);
}
