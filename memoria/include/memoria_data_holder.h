#include <commons/log.h>
#include <commons/collections/list.h>

typedef struct t_tabla_nivel_1 t_tabla_nivel_1;
typedef struct t_tabla_nivel_2 t_tabla_nivel_2;
typedef struct t_proceso_suspendido t_proceso_suspendido;
typedef struct t_marcos t_marcos;
typedef struct t_memoria_config t_memoria_config;

typedef struct t_memoria_data_holder {
    t_log* memoriaLogger;
    t_memoria_config* memoriaConfig;
    void* memoriaPrincipal;
    void* inicio_archivo;
    int archivo_swap;
    t_tabla_nivel_1* tablasDeNivel1;
    t_tabla_nivel_2* tablasDeNivel2;
    t_list* tablaSuspendidos;
    t_marcos* marcos;
    int tamanioPagina;
    int entradasPorTabla;
    int cantidadProcesosMax;
    int cantidadMarcosMax;
    int cantidadMarcosProceso;
    char* pathSwap;
    int contadorTabla1;
    int retardoSwap;
} t_memoria_data_holder;
