# HEA

O HEA é um algoritmo evolutivo híbrido  desenvolvido para solucionar o problema do escalonamento de tarefas e alocações de dados de workflows científicos.

Esse trabalho é uma nova abordagem sobre o trabalho desenvolvido por Luan Teylo no seguinte repositorio: https://github.com/luanteylo/HEA

## Instalação

Para executar o algoritmo é necessário instalar as dependências. Essa tarefa é facilmente executada utilizando o script [install.sh](https://github.com/luanteylo/HEA/blob/master/setup/install.sh) disponível na pasta **HEA/setup/**.

Para compilar o projeto utilize o arquivo [compile.sh](https://github.com/luanteylo/HEA/blob/master/compile.sh) 

## Arquivos de Entrada

Os arquivos de entrada podem ser encontrados na pasta **HEA/input**

## Execução

O arquivo executável se encontra na basta **bin/**

`cd HEA/bin`

`./HEA -c ../input/cluster.vcl -w ../input/miniworkflow.dag  -v`

A entrada é composta por dois arquivos: 

* O arquivo *.vcl* descreve o ambiente de execução e os recursos computacionais de cada máquina deste ambiente.

* O arquivo *.dag* descreve o workflow que deverá ser escalonado.


## Versioning

versão 1.0 - 2018.
