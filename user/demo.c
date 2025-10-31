#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NUM_PROCESOS 10
#define ITERACIONES_TRABAJO 1000000

// Realizar trabajo intensivo de CPU
void hacer_trabajo(int iteraciones) {
  volatile int suma = 0;
  for (int i = 0; i < iteraciones; i++) {
    suma += i;
  }
}

int main(int argc, char *argv[]) {
  int i;
  
  printf("Demo de Planificacion por Loteria\n");
  printf("Creando %d procesos con diferentes valores de tickets\n\n", NUM_PROCESOS);
  
  for (i = 0; i < NUM_PROCESOS; i++) {
    int pid = fork();
    
    if (pid < 0) {
      printf("fork fallo\n");
      exit(1);
    }
    
    if (pid == 0) {
      // Proceso hijo
      int tickets = 50 * (i + 1);  // 50, 100, 150, ..., 500 tickets
      settickets(tickets);
      
      printf("Proceso %d: PID=%d, Tickets=%d iniciado\n", i, getpid(), tickets);
      
      // Hacer algo de trabajo
      int contador_trabajo = 0;
      for (int j = 0; j < 5; j++) {
        hacer_trabajo(ITERACIONES_TRABAJO);
        contador_trabajo++;
      }
      
      printf("Proceso %d: PID=%d, Tickets=%d completo %d unidades de trabajo\n", 
             i, getpid(), tickets, contador_trabajo);
      exit(0);
    }
  }
  
  // Padre espera a todos los hijos
  printf("\nPadre esperando a que terminen todos los hijos...\n\n");
  for (i = 0; i < NUM_PROCESOS; i++) {
    wait(0);
  }
  
  printf("\nTodos los procesos completados.\n");
  printf("Comportamiento esperado: Procesos con mas tickets deberian ejecutarse mas frecuentemente.\n");
  printf("Proceso 9 (500 tickets) deberia obtener ~10x mas CPU que Proceso 0 (50 tickets).\n");
  
  exit(0);
}
