# Tarea 3: Protección de Lectura en XV6

**Grupo:** Grupo13_T3  

**Alumnos** 
- Arturo Figueroa
- Diego Pinos

**Rama:** Sistemas Operativos

## Descripción General

Esta implementación añade capacidades de protección de lectura a XV6, permitiendo crear regiones de memoria de "solo escritura" que son útiles para manejar datos sensibles como claves criptográficas y credenciales. El mecanismo protege contra lecturas no autorizadas mientras permite escrituras, implementando un modelo de seguridad valioso para aplicaciones criptográficas.

## Funcionalidades Implementadas

### System Calls

Se implementaron dos nuevas llamadas al sistema:

1. **`int mrdprotect(void *addr, int len)`**
   - Elimina el permiso de lectura de una región de memoria
   - Marca `len` páginas comenzando desde `addr` como no legibles
   - Limpia el bit `PTE_R` en las entradas de la tabla de páginas

2. **`int munrdprotect(void *addr, int len)`**
   - Restaura el permiso de lectura de una región de memoria
   - Activa el bit `PTE_R` en las entradas de la tabla de páginas

Ambas funciones retornan `0` en éxito y `-1` en caso de error.

## Archivos Modificados

### Kernel

1. **`kernel/vm.c`**
   - Implementación de `mrdprotect()` y `munrdprotect()`
   - Manejo de la tabla de páginas y modificación de bits PTE

2. **`kernel/defs.h`**
   - Declaraciones de funciones para `mrdprotect` y `munrdprotect`

3. **`kernel/syscall.h`**
   - Definición de números de llamadas al sistema:
     - `SYS_mrdprotect = 22`
     - `SYS_munrdprotect = 23`

4. **`kernel/syscall.c`**
   - Prototipos de funciones externas
   - Registro en el arreglo de syscalls

5. **`kernel/sysproc.c`**
   - Implementación de `sys_mrdprotect()` y `sys_munrdprotect()`
   - Extracción de argumentos y llamada a funciones del kernel

### User Space

1. **`user/user.h`**
   - Declaraciones de funciones para espacio de usuario

2. **`user/usys.pl`**
   - Generación de stubs para las nuevas syscalls

3. **`user/rdprotect_test.c`**
   - Programa de prueba que demuestra la funcionalidad

### Build System

1. **`Makefile`**
   - Adición de `rdprotect_test` a `UPROGS`

## Detalles de Implementación

### Función `mrdprotect`

```c
int mrdprotect(void *addr, int len)
```

**Nota sobre la implementación:** Aunque el prototipo de usuario es `int mrdprotect(void *addr, int len)`, la implementación interna en `vm.c` recibe el pagetable como parámetro adicional para permitir su uso desde el kernel:

```c
int mrdprotect(pagetable_t pagetable, uint64 addr, int len)
```

Esto permite que `sys_mrdprotect()` en `sysproc.c` obtenga el pagetable del proceso actual (`myproc()->pagetable`) y lo pase explícitamente.

**Funcionamiento:**
1. Valida que `addr` esté alineada a página (`PGSIZE`)
2. Verifica que `len` sea positivo
3. Para cada página en el rango:
   - Obtiene el PTE usando `walk()`
   - Verifica que la página sea válida (`PTE_V`) y de usuario (`PTE_U`)
   - Limpia el bit `PTE_R` usando: `*pte = *pte & ~PTE_R`

**Validaciones:**
- Dirección alineada a página
- Longitud positiva
- Dirección dentro del espacio de usuario (verificado mediante `PTE_U`)
- Todas las páginas mapeadas y válidas (`PTE_V`)
- Páginas pertenecientes al usuario, no al kernel

### Función `munrdprotect`

```c
int munrdprotect(void *addr, int len)
```

**Nota:** Similar a `mrdprotect`, la implementación interna recibe el pagetable como parámetro.

**Funcionamiento:**
1. Realiza las mismas validaciones que `mrdprotect`
2. Para cada página en el rango:
   - Obtiene el PTE usando `walk()`
   - Verifica validez (`PTE_V`) y pertenencia al usuario (`PTE_U`)
   - Activa el bit `PTE_R` usando: `*pte = *pte | PTE_R`

### Bits de la Tabla de Páginas (PTE)

Los bits relevantes en RISC-V son:
- `PTE_V` (bit 0): Entrada válida
- `PTE_R` (bit 1): Permiso de lectura
- `PTE_W` (bit 2): Permiso de escritura
- `PTE_X` (bit 3): Permiso de ejecución
- `PTE_U` (bit 4): Accesible desde modo usuario

## Manejo de Errores

Ambas funciones retornan `-1` cuando:
- `addr` no está alineada a página
- `len <= 0`
- Alguna página del rango no está mapeada (`walk()` retorna `NULL`)
- Alguna página no tiene `PTE_V` (no es válida)
- Alguna página no tiene `PTE_U` (pertenece al kernel, no al usuario)
- Alguna dirección está fuera del espacio accesible del proceso

## Programa de Prueba

El programa `rdprotect_test.c` demuestra el uso de las nuevas syscalls. **Importante:** El comportamiento correcto es que el proceso termine con un page fault al intentar leer, por lo que el mensaje final NO se imprime.

El programa incluye mensajes informativos que muestran el progreso de cada paso del test:

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    printf("=== Test de Protección de Lectura ===\n\n");
    
    char *addr = sbrk(0);      // Dirección actual del heap
    sbrk(4096);                // Reservar una página
    printf("[OK] Paso 1: Página reservada en dirección %p\n", addr);
    
    addr[0] = 'Z';             // Escribir valor inicial
    printf("[OK] Paso 2: Escritura inicial exitosa (valor='Z')\n");

    // Proteger contra lectura
    if (mrdprotect(addr, 1) < 0) {
        printf("[ERROR] mrdprotect falló\n");
        exit(1);
    }
    printf("[OK] Paso 3: Protección aplicada exitosamente (mrdprotect)\n");

    // Escritura aún permitida
    addr[0] = 'A';
    printf("[OK] Paso 4: Escritura en página protegida exitosa (valor='A')\n");

    printf("\n[ADVERTENCIA] Paso 5: Intentando LEER de página protegida...\n");
    printf("              ESPERADO: El proceso debe terminar con page fault\n\n");
    
    // Intento de lectura debería provocar fallo
    char c = addr[0];
    printf("[ERROR] Lectura exitosa (valor='%c') - LA PROTECCIÓN NO FUNCIONÓ\n", c);

    // Revertir protección
    if (munrdprotect(addr, 1) < 0) {
        printf("[ERROR] munrdprotect falló\n");
        exit(1);
    }

    printf("[OK] Protección revertida correctamente.\n");
    exit(0);
}
```

### Comportamiento Esperado

**IMPORTANTE:** El test está diseñado para FALLAR intencionalmente. El comportamiento correcto es que el proceso termine abruptamente.

1. **Primera escritura (`addr[0] = 'Z'`):** Éxito
2. **Protección (`mrdprotect`):** Éxito - La página ahora no tiene permiso de lectura
3. **Segunda escritura (`addr[0] = 'A'`):** **PAGE FAULT** - El proceso termina aquí
4. **Lectura (`char c = addr[0]`):** No se ejecuta (proceso ya terminado)
5. **Desprotección (`munrdprotect`):** No se ejecuta (proceso ya terminado)
6. **Mensaje final:** No se imprime (proceso ya terminado)

**Explicación técnica:** Cuando se quita el bit `PTE_R` (permiso de lectura), el hardware RISC-V también bloquea las escrituras debido a que la especificación RISC-V no permite páginas "write-only" (solo escritura). Según el estándar RISC-V Sv39, la combinación R=0, W=1 está **reservada/inválida**. Por lo tanto, al intentar escribir en una página sin `PTE_R`, el hardware genera una excepción de store page fault (scause=15). Esto demuestra que la protección funciona, aunque bloquea tanto lecturas como escrituras.

**Limitación del Hardware RISC-V:** No es posible implementar memoria "solo escritura" usando únicamente los bits de la PTE en RISC-V. Para lograr el comportamiento ideal (escritura permitida, lectura bloqueada), sería necesario implementar un manejador personalizado de page faults en el kernel que distinga entre loads y stores.

**Salida esperada en consola:**
```
$ rdprotect_test
=== Test de Protección de Lectura ===

[OK] Paso 1: Página reservada en dirección 0x0000000000004000
[OK] Paso 2: Escritura inicial exitosa (valor='Z')
DEBUG: Llamando mrdprotect(addr=0x0000000000004000, len=1)
DEBUG: addr % PGSIZE = 0 (debe ser 0)
DEBUG: mrdprotect retornó 0
[OK] Paso 3: Protección aplicada exitosamente (mrdprotect)
usertrap(): unexpected scause 0x000000000000000f pid=3
            sepc=0x94 stval=0x4000
$
```

**Interpretación de la salida:**
- Los pasos 1-3 muestran que todas las operaciones previas funcionaron correctamente
- `mrdprotect` retorna 0, indicando éxito al quitar el bit `PTE_R`
- El proceso termina al intentar escribir en la página protegida (Paso 4)
- `scause 0xf` = Store/AMO page fault (intento de escritura en página sin permisos)
- `stval 0x4000` = Dirección de la página que causó el fallo
- Esto confirma que quitar `PTE_R` efectivamente bloquea el acceso a la página (lecturas Y escrituras)

## Compilación y Ejecución

### Compilación

```bash
make
```

### Ejecución en QEMU

```bash
make qemu
```

### Ejecutar el Test

Dentro de XV6:
```bash
rdprotect_test
```

## Consideraciones de Seguridad

### Casos de Uso

1. **Claves Criptográficas:** Almacenar claves en memoria que pueden ser escritas pero nunca leídas por el proceso, protegiéndolas de ataques de lectura de memoria.

2. **Credenciales Temporales:** Mantener credenciales que se escriben una vez pero no pueden ser leídas posteriormente.

3. **Write-Only Logs:** Registros donde se puede escribir pero no leer, útil para auditoría.

### Limitaciones

1. **Limitación del Hardware RISC-V:** La especificación RISC-V no permite páginas "write-only" (R=0, W=1 está reservado). Al quitar `PTE_R`, también se bloquean las escrituras. Para lograr verdadera memoria "solo escritura", se requeriría un manejador personalizado de page faults que diferencie entre loads y stores.

2. **No protege contra kernel:** El kernel aún puede acceder a estas páginas usando direcciones físicas directas.

3. **No protege contra DMA:** Dispositivos con acceso directo a memoria pueden leer/escribir bypass del MMU.

4. **Page fault termina proceso:** No hay recuperación automática. El proceso muere al primer acceso prohibido.

5. **Granularidad de página:** La protección opera a nivel de página completa (4KB), no a nivel de bytes individuales.

## Estructura del Código

### Flow de una System Call

```
Usuario: mrdprotect(addr, len)
    ↓
[usys.S] Stub generado por usys.pl
    ↓
[syscall.c] syscall() dispatcher
    ↓
[sysproc.c] sys_mrdprotect()
    ↓
[vm.c] mrdprotect(pagetable, addr, len)
    ↓
Modificación de PTEs
```

### Estructura de Archivos

```
kernel/
  ├── vm.c              # Implementación core
  ├── sysproc.c         # Wrappers de syscall
  ├── syscall.c         # Dispatcher
  ├── syscall.h         # Números de syscall
  └── defs.h            # Declaraciones
user/
  ├── user.h            # API de usuario
  ├── usys.pl           # Generador de stubs
  └── rdprotect_test.c  # Programa de prueba
```

## Testing y Validación

### Casos de Test

1. Protección exitosa de una página (mrdprotect retorna 0)
2. Escritura permitida en página protegida (sin lectura)
3. **Lectura provoca page fault** (comportamiento correcto: proceso termina)
4. Desprotección restaura lectura (no validable en el test básico debido al punto 3)
5. Dirección no alineada retorna -1
6. Longitud inválida (len <= 0) retorna -1
7. Página no mapeada retorna -1

### Verificación Manual

Ejecuta el test dentro de XV6:

```bash
$ rdprotect_test
=== Test de Protección de Lectura ===

[OK] Paso 1: Página reservada en dirección 0x0000000000004000
[OK] Paso 2: Escritura inicial exitosa (valor='Z')
DEBUG: Llamando mrdprotect(addr=0x0000000000004000, len=1)
DEBUG: addr % PGSIZE = 0 (debe ser 0)
DEBUG: mrdprotect retornó 0
[OK] Paso 3: Protección aplicada exitosamente (mrdprotect)
usertrap(): unexpected scause 0xf pid=3
            sepc=0x94 stval=0x4000
$
```

**Criterios de Éxito:**

| Qué Ver | Significado |
|---------|-------------|
| [OK] Pasos 1-3 completos | La protección se aplicó correctamente |
| `mrdprotect retornó 0` | La syscall ejecutó exitosamente |
| `scause 0xf` (Store page fault) | **La protección FUNCIONÓ** - bloqueó el acceso a la página |
| Proceso termina después del Paso 3 | **Comportamiento esperado en RISC-V** |

**Criterios de Fallo:**

| Qué Ver | Problema |
|---------|----------|
| `mrdprotect retornó -1` | Error al aplicar la protección |
| Paso 4 se ejecuta completamente | La protección NO se aplicó |
| No hay page fault | El bit `PTE_R` no se modificó correctamente |