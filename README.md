# Visualizador 3D - Trabalho GA - Processamento Gráfico: Computação Gráfica e Aplicações

## Desenvolvido por: Gustavo Lavina e Vitor Goulart

## Como executar

- Clonar repositório.
- Recomendado: abrir pelo Visual Studio Code, tendo instalado as extensões C/C++ e CMake Tools, ambas da Microsoft.
- Ao abrir, um dropdown na aba superior da tela permitirá escolher o compilador a ser utilizado. Escolha amd64.
- Para executar, basta clicar em Build e, depois, no botão de executar, ambos na parte inferior esquerda da tela. Os processos de configuração do CMake e de Build podem demorar um pouco, pois o CMake estará instalando as bibliotecas GLFW e GLM.

- Caso ocorra erro na leitura dos arquivos de shader ou dos arquivos .obj, verifique se o caminho destes arquivos no main.cpp está correspondendo com o caminho relativo do .exe até os arquivos em questão. O CMake pode produzir .exe com locais diferentes dependendo do ambiente, exigindo que os caminhos relativos sejam ajustados no código fonte.

## Carregamento da cena a partir do arquivo JSON

- O arquivo `scene.json` já está no modelo correto de configuração da cena;
- Não implementamos a capacidade de carregar objetos com mais de um grupo de malhas;
- Não implementamos a possibilidade de inserir mais de uma fonte de luz;
- Os valores Ka, Ks e Kd de um objeto lido no arquivo JSON são substituídos pelos valores do arquivo MTL se existirem;

## Controles

### Controles do objeto

- Rotação em X: 'X'
- Rotação em Y: 'Y'
- Rotação em Z: 'Z'

- Translação em X: 'D' e 'A'
- Translação em Y: 'W' e 'S'
- Translação em Z: 'Q' e 'E'

- Escala (todos os eixos): HOME e END

- Seleção de objeto: ',' e '.'

### Controles da camera

- Rotação em X e Y: Mouse
- Translação em X: LEFT e RIGHT
- Translação em Y: PAGE_UP e PAGE_DOWN
- Translação em Z: UP e DOWN
