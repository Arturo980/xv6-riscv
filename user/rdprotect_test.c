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
    if (mrdprotect(addr, 1) < 0) {
        printf("✗ ERROR: mrdprotect falló\n");
        exit(1);
    }
    printf("✓ Paso 3: Protección aplicada exitosamente (mrdprotect)\n");

    // Escritura aún permitida
    addr[0] = 'A';
    printf("✓ Paso 4: Escritura en página protegida exitosa (valor='A')\n");

    printf("\n⚠️  Paso 5: Intentando LEER de página protegida...\n");
    printf("    ESPERADO: El proceso debe terminar con page fault\n\n");
    
    // Intento de lectura debería provocar fallo
    char c = addr[0];
    printf("✗ ERROR: Lectura exitosa (valor='%c') - LA PROTECCIÓN NO FUNCIONÓ\n", c);

    // Revertir protección
    if (munrdprotect(addr, 1) < 0) {
        printf("✗ ERROR: munrdprotect falló\n");
        exit(1);
    }

    printf("✓ Protección revertida correctamente.\n");
    exit(0);
}
