#include <stdio.h>
#include <stdlib.h>

#include "signal_processing.h"


extern int cuentaP25;
extern int campo;
extern double numeros1[1000];
extern double numeros2[1000];
extern double t[1000];

static int comparar_picos(const void *a, const void *b) {
    Pico *picoA = (Pico *)a;
    Pico *picoB = (Pico *)b;

    if (campo == 1) {
        if (picoA->valor < picoB->valor) return 1;
        if (picoA->valor > picoB->valor) return -1;
    } else if (campo == 0) {
        if (picoA->tiempo < picoB->tiempo) return 1;
        if (picoA->tiempo > picoB->tiempo) return -1;
    }
return 0;
}


Pico* detectarP25(double *signal,double *time,int tamano){
   // changeSign(signal);
    ResultadoPicos resultado = encontrar_picos(signal,time,tamano);
    if (resultado.picos == NULL) {
            return NULL;
        }
    Pico *filtered_picos = (Pico *)malloc(resultado.num_picos * sizeof(Pico));
    if (filtered_picos == NULL) {
        fprintf(stderr, "Error: no se pudo reservar memoria para los picos filtrados\n");
        free(resultado.picos);
        return NULL;
    }
    int filtered_count = 0;
    for (int i = 0; i < resultado.num_picos; i++) {
        if (resultado.picos[i].tiempo >= 19.5 && resultado.picos[i].tiempo <= 32.0) {
            filtered_picos[filtered_count++] = resultado.picos[i];
        }
    }
    Pico *picos_selec = NULL;
    if (filtered_count > 0) {
    	campo=1;
        qsort(filtered_picos, filtered_count, sizeof(Pico), comparar_picos);
        picos_selec = (Pico *)malloc(3 * sizeof(Pico));
        if (picos_selec == NULL) {
            fprintf(stderr, "Error: no se pudo reservar memoria para los picos seleccionados\n");
            free(filtered_picos);
            free(resultado.picos);
            return NULL;
        }

    printf("Se ha detectado la onda P25. Las latencias y amplitudes son:\n");
    for (int i = 0; i < 3 && i < filtered_count; i++) {
            picos_selec[i]=filtered_picos[i];
            cuentaP25++;
            printf("Latencia: %.4f, Amplitud: %.7f\n", picos_selec[i].tiempo, picos_selec[i].valor);
    }
    } else {
        printf("No se ha detectado la onda P25\n");
    }

    free(filtered_picos);
    free(resultado.picos);
    return picos_selec;
}

Pico* detectarN20(double *signal,double *time,int tamano,Pico* picosP25) {
    changeSign(signal);
    ResultadoPicos resultado = encontrar_picos(signal,time,tamano);
    if (resultado.picos == NULL) {
          return NULL;
      }
    Pico *filtered_picos = (Pico *)malloc(resultado.num_picos * sizeof(Pico));
    if (filtered_picos == NULL) {
        fprintf(stderr, "Error: no se pudo reservar memoria para los picos filtrados\n");
        free(resultado.picos);
        return NULL;
    }
    int filtered_count = 0;

    for (int i = 0; i < resultado.num_picos; i++) {
    	if (resultado.picos[i].tiempo >= 15.5 && resultado.picos[i].tiempo <= 29.0) {
    		filtered_picos[filtered_count++] = resultado.picos[i];
            printf("Pico %d: Valor = %.7f, Tiempo = %.7f\n", i + 1, resultado.picos[i].valor, resultado.picos[i].tiempo);
        }
    }


    if (filtered_count > 0) {
    	campo=1;
        qsort(filtered_picos, filtered_count, sizeof(Pico), comparar_picos);
    }
    if (cuentaP25 > 0) {
    	campo=0;
                qsort(picosP25, cuentaP25, sizeof(Pico), comparar_picos);
            }
        Pico maxN20=filtered_picos[0];
        Pico maxP25= picosP25[0];
        for (int i = 0; i < filtered_count; i++) {
        		printf("Pico  Valor = 7f, Tiempo =f\n");

        }
        for (int i = 0; i < filtered_count; i++) {
            if (filtered_picos[i].tiempo < maxP25.tiempo){
            maxN20=filtered_picos[i];
            break;
            }
        }

    Pico *picosP25_selec= (Pico *)malloc(3 * sizeof(Pico));
    if (picosP25_selec == NULL) {
            fprintf(stderr, "Error: no se pudo reservar memoria para los picos seleccionados\n");
            free(filtered_picos);
            free(resultado.picos);
            return NULL;
        }
    int num=0;
    for (int i = 0; i < cuentaP25; i++) {
        if (picosP25[i].tiempo > (maxN20.tiempo + 0.5)){
        picosP25_selec[num++]=picosP25[i];

        }
    }
    int indice=0;
    double minimo=picosP25_selec[0].tiempo- maxN20.tiempo;
     for (int i = 1; i < num; i++){
    	 double diff = picosP25_selec[i].tiempo - maxN20.tiempo;
        if (diff < minimo) {
            minimo=diff;
            indice=i;
        }
     }

    Pico P25=picosP25_selec[indice];
    maxN20.valor = maxN20.valor + P25.valor;
    Pico *res= (Pico *)malloc(3 * sizeof(Pico));
    if (res == NULL) {
            fprintf(stderr, "Error: no se pudo reservar memoria para los picos seleccionados\n");
            free(filtered_picos);
            free(picosP25_selec);
            free(resultado.picos);
            return NULL;
        }
    res[0]=P25;
    res[1]=maxN20;


    free(filtered_picos);
    free(picosP25_selec);
    free(resultado.picos);

     if ((maxN20.tiempo <=23) && (maxN20.tiempo >= 16) && (maxN20.valor >=0.6)){
        printf("Se ha detectado la onda N20. La latencia es: %.4f y la amplitud es: %.7f\n", maxN20.tiempo,maxN20.valor);
        res[2].valor=1;
        return res;
     }
    else {
        printf("No se ha detectado la onda N20");
        res[2].valor=0;
        return res;

    }
}


ResultadoPicos encontrar_picos(double* datos, double* tiempos, int tamano) {
    ResultadoPicos resultado;
    resultado.num_picos = 0;
    resultado.picos = (Pico*)malloc(tamano * sizeof(Pico));
    if (resultado.picos == NULL) {
        fprintf(stderr, "Error: no se pudo reservar memoria para el array de picos\n");
        resultado.num_picos = -1;
        return resultado;
    }
    int r;
    for (int i = 1; i < tamano - 1; i++) {

    	//int l=1; 
    	r=1;
    	//while(i - l >= 0 && datos[i - l] == datos[i]){
    		//l++;
    	//}
    	while(i + r < tamano && datos[i + r] == datos[i]){
    	    r++;
    	    	}
        if ((i + r < tamano) && datos[i] > datos[i - 1] && datos[i] > datos[i + r]) {
            //printf("pico: %i %i %.11f %.11f %.11f ", num_picos,i,datos[i], datos[i-1], datos[i+1]);
            resultado.picos[resultado.num_picos].valor = datos[i];
            resultado.picos[resultado.num_picos].tiempo = tiempos[i];
            resultado.num_picos++;
        }
        i=i + r - 1;
    }
    resultado.picos = (Pico*)realloc(resultado.picos, resultado.num_picos * sizeof(Pico));
    if (resultado.picos == NULL && resultado.num_picos > 0) {
        fprintf(stderr, "Error: no se pudo redimensionar el array de picos\n");
        resultado.num_picos = -1;
    }
    return resultado;
}


void changeSign(double *vector) {
     if (vector == NULL) {
        printf("el archivo esta vacio.");
        return;
    }
    //int size = sizeof(vector) / sizeof(vector[0]);
    for (int i = 0; i < 1000; i++) {
        vector[i] = -vector[i];
    }
    printf("signo cambiado");
} 

int algoritmo(void) {


    //changeSign(numeros1);
    //changeSign(numeros2);
    Pico* picos_detec1= detectarP25(numeros1, t, 1000);
    if (picos_detec1 == NULL) {
        printf("No se detectaron picos.\n");
        return 2;
    }
/*
    double valor=0;
    double tiempo=0;
    for (int i=0; i<3;i++){
    	valor=picos_detec1[i].valor;
    	tiempo=picos_detec1[i].tiempo;
    }*/
    Pico* detec1N20=detectarN20(numeros1,t,1000,picos_detec1);
	free(picos_detec1);
	if (detec1N20 == NULL) {
		printf("No se detectaron picos.\n");
		return 3;
	}

	cuentaP25=0;
    Pico* picos_detec2= detectarP25(numeros2, t, 1000);
    if (picos_detec2 == NULL) {
        printf("No se detectaron picos.\n");
        free(detec1N20);
        return 2;
    }

    Pico* detec2N20=detectarN20(numeros2,t,1000,picos_detec2);
    free(picos_detec2);

     if (detec2N20 == NULL) {
        printf("No se detectaron picos.\n");
        free(detec1N20);
        return 3;
    }

    if (detec1N20[2].valor != detec2N20[2].valor) {
        printf("Prueba inconclusa, por favor vuelva a repetir la prueba\n");
        free(detec1N20);
        free(detec2N20);
        return 4;
    } else if (detec1N20[2].valor == 1) {
        printf("Se ha detectado la onda N20.\n");
        free(detec1N20);
        free(detec2N20);
        return 5;
    } else {
        printf("No se ha detectado la onda N20\n");
        free(detec1N20);
        free(detec2N20);
        return 6;
    }


    return 0;
}
