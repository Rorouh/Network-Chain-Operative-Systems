# SOchain – Fase 1

## Información del Grupo  
- **Grupo:** 054  
- **Integrantes:**  
  - Miguel Ángel López Sánchez (fc64675)  
  - Alejandro Domínguez (fc64447)  
  - Bruno Felisberto (fc32435)

## Descripción del Proyecto  
Este proyecto consiste en desarrollar una aplicación en C llamada **SOchain**, que simula un sistema de transacciones para un token criptográfico denominado **SOT**. La idea es familiarizarnos con problemas de gestión de procesos, memoria dinámica y compartida, y comunicación entre procesos.

La aplicación está dividida en tres módulos principales:  
- **Main:** Se encarga de la interacción con el usuario, recibe comandos, crea transacciones y muestra información (como saldos y recibos).  
- **Wallets:** Representan las carteras; cada proceso wallet se encarga de firmar las transacciones cuyo origen le corresponde.  
- **Servers:** Son los procesos que validan, procesan y registran las transacciones, actualizando los saldos y generando recibos.

El flujo es el siguiente:  
1. El usuario ingresa un comando en la interfaz (por ejemplo, para crear una transacción).  
2. Main crea la transacción y la coloca en un buffer compartido con las wallets.  
3. La wallet correspondiente lee la transacción, la firma y la reenvía a un buffer circular hacia los servers.  
4. Los servers validan y procesan la transacción, actualizando los saldos y enviando un recibo de vuelta a Main.

## Estructura del Proyecto

```
SOCHAIN/
├── bin/                # Ejecutables generados (SOchain)
├── inc/                # Archivos de cabecera (.h)
├── obj/                # Archivos objeto (.o)
├── src/                # Archivos fuente (.c)
├── makefile            # Instrucciones de compilación
└── README.md           # Este documento
```

## Instrucciones de Compilación y Ejecución  
Para compilar el proyecto, hay que abrir una terminal en la raíz del proyecto y ejecuta:

```bash
make
```

Para ejecutar el programa con unos argumentos por defecto (saldo 100.0, 3 carteras, 2 servidores, tamaño de buffer 10 y máximo de 50 transacciones), ejecuta:

```bash
make run
```

*Nota:* Estos argumentos se pueden modificar si queremos.

## Comandos Disponibles en SOchain  
- **bal id:** Muestra el saldo de la cartera con identificador *id*.  
- **trx src_id dest_id amount:** Crea una transacción en la que la cartera *src_id* envía *amount* SOT a la cartera *dest_id*.  
- **rcp id:** Muestra el recibo de la transacción con identificador *id*.  
- **stat:** Imprime estadísticas actuales del sistema (configuración, contadores, PIDs, saldos, etc.).  
- **help:** Muestra los comandos disponibles.  
- **end:** Termina la ejecución de SOchain, imprime las estadísticas finales y cierra los procesos.

## Limitaciones del Trabajo  
- **Sin sincronización:** En esta fase no se han implementado mecanismos de sincronización entre procesos.
- **Manejo básico de errores:** El control de errores es muy importante, por ejemplo, la validación de parámetros y la gestión de fallos en la comunicación entre procesos.
- **Registro de transacciones:** Actualmente, las transacciones se muestran en pantalla en lugar de almacenarse en un alguna billetera real.

