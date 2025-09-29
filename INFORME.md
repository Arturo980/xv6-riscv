# Informe - Tarea 1: Llamadas al Sistema en xv6

Alumnos:

- Arturo Figueroa
- Diego Pinos

## Descripción

En esta tarea se implementaron dos nuevas llamadas al sistema dentro de xv6:

* `getppid()`: devuelve el PID del proceso padre.
* `getancestor(int n)`: devuelve el PID del ancestro *n*-ésimo del proceso actual.

## Funcionamiento

### `getppid()`

La función retorna el PID del proceso padre accediendo al campo `parent` de la estructura `proc`. En caso de que no exista un padre, retorna -1.

### `getancestor(int n)`

Se recorre la cadena de ancestros hasta llegar al *n*-ésimo. Si `n=0` se retorna el PID del mismo proceso, si `n=1` el del padre, y así sucesivamente. Si no existe un ancestro válido o si `n` es menor que 0, retorna -1.

## Cambios Realizados en xv6

* **kernel/syscall.h** - Se agregaron los números de las nuevas syscalls.
* **kernel/syscall.c** - Se incluyeron las funciones y entradas en el arreglo de syscalls.
* **kernel/sysproc.c** - Se implementaron las funciones `sys_getppid` y `sys_getancestor`.
* **user/user.h** - Se agregaron las declaraciones de las funciones a nivel de usuario.
* **user/usys.pl** - Se añadieron las entradas para los stubs.
* **Makefile** - Se agregó el programa de prueba en `UPROGS`.
* **user/yosoytupadre.c** - Programa de prueba para validar las dos llamadas.

## Programa de Prueba


El programa `yosoytupadre.c` prueba diferentes escenarios:

**En el proceso principal:**
- Se imprime el PID y el PPID para mostrar la relación básica entre el proceso y su padre.
- Para mostrar los ancestros, se utiliza un bucle `for` que recorre los valores de `n` desde 0 hasta 3. Esto permite ver fácilmente cómo la llamada `getancestor(n)` retorna el PID del propio proceso, el padre, el abuelo y el bisabuelo, respectivamente. Usar un bucle hace el código más compacto y flexible, ya que si se quisiera probar más niveles, solo habría que cambiar el rango del for.
- Además, se imprime el resultado de `getancestor(10)`. Este caso sirve para mostrar qué ocurre cuando se pide un ancestro que no existe (por ejemplo, si el proceso no tiene tantos padres en la cadena). Así se verifica que la función retorna -1 correctamente en situaciones fuera del rango esperado.

**Casos especiales:**
- Se prueba también con valores negativos para confirmar que la función retorna -1 cuando el parámetro no es válido.

## Dificultades Encontradas

* **Integrar el Makefile**: Hubo que verificar que el programa de usuario quedara bien compilado e incluido en la imagen de xv6.
* **Acceso a ancestros**: Fue importante manejar los casos límite para que no ocurrieran errores si no existían más padres.
* **Pruebas**: Revisar cuidadosamente la salida en distintos escenarios de procesos para confirmar que las llamadas estaban funcionando como se esperaba.

## Ejecución

Para probar la tarea se deben seguir los pasos:

1. Compilar y ejecutar xv6:

   ```bash
   make clean
   make
   make qemu
   ```
2. En la consola de xv6 correr:

   ```
   yosoytupadre
   ```

## Conclusión

Las llamadas al sistema se implementaron con éxito y el programa de prueba permitió comprobar su correcto funcionamiento en todos los casos planteados, incluyendo situaciones normales y límites.