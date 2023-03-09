# Laboratorio 1 - Mybash: Programando nuestro propio shell de Linux

## Qué funciona
- Comandos simples con argumentos
- Pipelines de dos comandos
- Ejecutar en background (`&`)
- Redirecciones (`>` y `<`)
- Comandos internos: `cd` y `exit`

## Qué no funciona
- Pipelines de mas de dos comandos
- Comandos secuenciales (`&&`, `;`, `||`, etc)

## Extras
- El prompt muestra la ruta actual
- `cd` cambia al directorio home cuando no se le da argumentos

## Comandos testeados
- `ls`
- `ls --color`
- `ls | wc`
- `ls | grep mybash`
- `ls | grep mybash > test.txt`
- `xeyes &`
- `ñsaj {}dfhwiuoyrtrjb23 b2 998374 2h231 #$L!,`
- `ls -l *.c >`
- `wc < test.txt`
- `cd tests`
- `cd`

## Dependencias
- glib2
- check<=0.14 (para testing)

## Cómo compilar
1. Clonar la repo
2. `cd so21lab1g33`
3. `make`

## Cómo ejecutar
`./mybash` en el directorio `so21lab1g33`

## Testing
- `make test-command` para testear los módulos scommand y pipeline
- `make test` para testear los módulos execute y builtin, además de los anteriormente dichos
- `make memtest` para buscar memory leaks
