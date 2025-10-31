# Tarea 2: Planificación de Procesos en XV6 - Lottery Scheduling

Alumnos:

- Arturo Figueroa
- Diego Pinos

## Descripción General

Este proyecto implementa un sistema de **Lottery Scheduling** (planificación por lotería) en el sistema operativo XV6-RISCV, reemplazando el planificador Round-Robin original. En este esquema, cada proceso recibe una cantidad de "tickets" y la probabilidad de que un proceso sea seleccionado para ejecutarse es proporcional al número de tickets que posee.

## Implementación

### 1. Funcionamiento y Lógica

El Lottery Scheduling funciona mediante los siguientes pasos:

1. **Asignación de Tickets**: Cada proceso tiene un campo `tickets` que determina su prioridad relativa. Más tickets = mayor probabilidad de ser elegido.

2. **Selección Aleatoria**: El scheduler:
   - Calcula el total de tickets de todos los procesos RUNNABLE
   - Genera un número aleatorio entre 1 y el total
   - Recorre los procesos acumulando tickets hasta alcanzar el número aleatorio
   - El proceso que hace que el acumulador supere el número aleatorio es el ganador

3. **Contabilidad**: El campo `run_slices` rastrea cuántas veces cada proceso ha sido seleccionado, permitiendo verificar la proporcionalidad.

### Algoritmo del Scheduler

```c
1. Contar total_tickets de procesos RUNNABLE
2. Si total_tickets == 0, esperar interrupción (WFI)
3. Generar random = (ticks * LCG) % total_tickets + 1
4. counter = 0
5. Para cada proceso RUNNABLE:
   a. counter += proceso.tickets
   b. Si counter >= random:
      - Marcar proceso como RUNNING
      - Incrementar proceso.run_slices
      - Ejecutar proceso (swtch)
      - Terminar búsqueda
```

### Generación de Números Aleatorios

Utilizamos un **Linear Congruential Generator (LCG)** simple basado en `ticks` (contador de interrupciones del reloj):

```c
random = (ticks * 1103515245 + 12345) % total_tickets + 1
```

Esto proporciona suficiente variación para la selección de procesos sin requerir un RNG complejo.

## Modificaciones Realizadas

### Archivos Modificados

#### 1. `kernel/proc.h`
**Cambios**: Agregado dos campos a `struct proc`:
```c
int tickets;      // Número de tickets de lotería
int run_slices;   // Contador de veces programado
```

#### 2. `kernel/proc.c`
**Cambios**:
- **`allocproc()`**: Inicializa `tickets = 100` y `run_slices = 0` para nuevos procesos
- **`scheduler()`**: Reemplazado el algoritmo Round-Robin completo con Lottery Scheduling
  - Primera pasada: contar tickets totales
  - Generación de número aleatorio
  - Segunda pasada: selección del proceso ganador
  - Incremento de `run_slices`

#### 3. `kernel/syscall.h`
**Cambios**: Agregado la definición de syscall:
```c
#define SYS_settickets 22
```

#### 4. `kernel/syscall.c`
**Cambios**:
- Agregado declaración: `extern uint64 sys_settickets(void);`
- Agregado entrada en el array `syscalls[]`: `[SYS_settickets] sys_settickets,`

#### 5. `kernel/sysproc.c`
**Cambios**: Implementado `sys_settickets()`:
```c
uint64 sys_settickets(void) {
  int n;
  struct proc *p = myproc();
  argint(0, &n);
  if(n < 1) n = 1;  // Mínimo 1 ticket
  acquire(&p->lock);
  p->tickets = n;
  release(&p->lock);
  return 0;
}
```

#### 6. `user/user.h`
**Cambios**: Agregado prototipo:
```c
int settickets(int);
```

#### 7. `user/usys.pl`
**Cambios**: Agregado entrada:
```perl
entry("settickets");
```

#### 8. `user/demo.c` (NUEVO)
**Cambios**: Programa de prueba que:
- Crea 10 procesos hijos
- Asigna tickets incrementales: 50, 100, 150, ..., 500
- Cada proceso realiza trabajo intensivo de CPU
- Imprime información de ejecución

#### 9. `Makefile`
**Cambios**: Agregado `$U/_demo\` a la lista UPROGS

## Dificultades y Soluciones

### Dificultad 1: Generación de Números Aleatorios
**Problema**: XV6 no tiene una función `rand()` incorporada.

**Solución**: Implementamos un LCG simple usando `ticks` como fuente de entropía. Aunque no es criptográficamente seguro, es suficiente para el propósito del scheduler.

### Dificultad 2: Sincronización y Locks
**Problema**: El scheduler debe acceder a múltiples procesos mientras mantiene la consistencia.

**Solución**: 
- Adquirir/liberar `p->lock` para cada proceso individualmente
- No mantener locks durante la generación del número aleatorio
- Dos pasadas sobre la tabla de procesos (una para contar, otra para seleccionar)

### Dificultad 3: Caso Edge - Sin Tickets
**Problema**: ¿Qué pasa si todos los procesos RUNNABLE tienen 0 tickets?

**Solución**: 
- `allocproc()` garantiza mínimo 100 tickets al crear
- `sys_settickets()` garantiza mínimo 1 ticket
- `scheduler()` verifica `total_tickets == 0` y entra en WFI

### Dificultad 4: Proporcionalidad en la Práctica
**Problema**: El comportamiento puede no ser perfectamente proporcional con pocos procesos.

**Solución**: 
- Agregamos `run_slices` para tracking estadístico
- Con suficiente tiempo de ejecución, la distribución converge a la proporcional

## Problemas del Lottery Scheduling

### 1. **Inanición (Starvation)**
**Descripción**: Un proceso con muy pocos tickets puede nunca ser seleccionado si hay muchos procesos con más tickets.

**Ejemplo**: Un proceso con 1 ticket compitiendo contra 100 procesos con 100 tickets cada uno (total 10,001 tickets) tiene solo 0.01% de probabilidad de ser elegido.

**Solución Potencial**: Implementar "aging" - incrementar tickets de procesos que no han ejecutado en mucho tiempo.

### 2. **Falta de Garantías de Tiempo Real**
**Descripción**: No hay garantías determinísticas sobre cuándo un proceso será ejecutado.

**Impacto**: Inadecuado para sistemas de tiempo real donde se requieren deadlines estrictos.

### 3. **Complejidad en Asignación de Tickets**
**Descripción**: Difícil determinar cuántos tickets asignar para lograr una proporción específica de CPU.

**Ejemplo**: Si quiero que un proceso use exactamente 30% del CPU, ¿cuántos tickets necesito? Depende de cuántos otros procesos existan y sus tickets.

### 4. **Overhead de Dos Pasadas**
**Descripción**: Nuestra implementación requiere dos recorridos de la tabla de procesos:
- Primera pasada: contar tickets totales
- Segunda pasada: seleccionar ganador

**Impacto**: O(n) tiempo en cada scheduling decision, peor que Round-Robin O(1) (con estructuras apropiadas).

**Solución Potencial**: Mantener un contador global de tickets que se actualice cuando cambian los tickets de un proceso.

### 5. **Calidad del Generador Aleatorio**
**Descripción**: Un mal RNG puede introducir sesgos en la selección.

**Nuestra Implementación**: El LCG basado en `ticks` puede tener patrones predecibles, especialmente con valores pequeños de `total_tickets`.

**Solución Potencial**: Usar un RNG más sofisticado (Xorshift, Mersenne Twister) o hardware RNG si está disponible.

### 6. **Ticket Inflation**
**Descripción**: En sistemas donde procesos pueden modificar sus propios tickets, podrían inflar artificialmente su prioridad.

**Nuestra Implementación**: Cualquier proceso puede llamar `settickets()` sin restricciones.

**Solución Potencial**: 
- Solo permitir al superusuario/kernel modificar tickets
- Limitar el máximo de tickets por proceso
- Implementar un sistema de "currency" donde los tickets consumidos se restan

### 7. **Problema de Tickets Compensatorios**
**Descripción**: Procesos que se bloquean frecuentemente (I/O) liberan sus tickets temporalmente, dando más CPU a procesos compute-bound.

**Impacto**: Proceso I/O-bound con 500 tickets puede recibir menos CPU que proceso compute-bound con 100 tickets si pasa 80% del tiempo esperando I/O.

**Solución Potencial**: "Ticket transfer" - transferir tickets a procesos en los que esperamos (ej: durante I/O).

## Compilación y Ejecución

### Compilar XV6:
```bash
make clean
make qemu
```

### Ejecutar Demo en XV6:
```
$ demo
```

### Salir de QEMU:
```
Ctrl-A X
```

## Pruebas

El programa `demo.c` crea 10 procesos con tickets: 50, 100, 150, 200, 250, 300, 350, 400, 450, 500.

**Comportamiento Esperado**:
- Proceso con 500 tickets debería ejecutarse ~10x más frecuentemente que el de 50 tickets
- Proceso con 500 tickets debería ejecutarse ~2x más frecuentemente que el de 250 tickets
- La distribución debería ser aproximadamente proporcional con suficiente tiempo de ejecución

**Observación**: Con el campo `run_slices`, podríamos agregar una syscall adicional para leer este valor y verificar numéricamente la proporcionalidad.

## Mejoras Futuras

1. **Syscall para leer estadísticas**: Agregar `getprocinfo()` para leer tickets y run_slices
2. **Implementar Stride Scheduling**: Alternativa determinística a Lottery
3. **Tickets jerárquicos**: Grupos de procesos que comparten un pool de tickets
4. **RNG mejorado**: Implementar un PRNG de mejor calidad
5. **Contador global optimizado**: Evitar el primer recorrido de procesos
6. **Debugging tools**: Agregar impresión de estadísticas del scheduler

## Referencias

- Waldspurger, C. A., & Weihl, W. E. (1994). "Lottery scheduling: Flexible proportional-share resource management"
- Documentación original de XV6: https://pdos.csail.mit.edu/6.828/
- RISC-V Privileged Architecture Specification

## Autor

Implementación realizada como parte del curso de Sistemas Operativos.

---


