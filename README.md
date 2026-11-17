# GTA SA Film Studio (GTA SA 1.0 US) - Etapas 1 e 2

Estado: codigo-fonte escrito, AINDA NAO COMPILADO nem testado em jogo (o ambiente de
desenvolvimento nao tem compilador Windows). Erros de compilacao sao esperados na 1a tentativa.

## Compilar (sem instalar nada): GitHub Actions
1. Crie um repositorio no GitHub e envie esta pasta (git push).
2. Aba Actions -> workflow "build" -> baixe o artefato `FilmStudio-asi` (FilmStudio.asi).
3. Se falhar, copie o log de erro e me envie.

## Compilar localmente (Visual Studio 2022 + CMake)
1. Clone https://github.com/DK22Pac/plugin-sdk, rode premake5 vs2022 e compile
   plugin_sa/plugin.vcxproj em Release|Win32 (gera output/lib/plugin.lib).
2. set PLUGIN_SDK_DIR=<pasta do plugin-sdk>
3. cmake -S . -B build -A Win32 && cmake --build build --config Release

## Instalar
Requisitos: GTA SA 1.0 US, Ultimate ASI Loader (dinput8.dll ou d3d9.dll de 32 bits;
o d3d9-x64.zip enviado NAO serve, e de 64 bits). Copie FilmStudio.asi para a pasta do jogo
(ou para scripts\\ / modloader\\). Abra o menu com F9 (mude em Settings). Log: FilmStudio.log.

## Estrutura
Todos os arquivos ficam na raiz (facilita upload no GitHub); veja NOTAS_E_PENDENCIAS.md.

## Etapas
1 Fundacao, 2 NPC Manager, 3 Position+Behavior, 4 Animation, 5 Textures, 6 Camera: codigo escrito, nao compilado/testado.
7 Integracao e testes: depende de voce compilar e testar no jogo.

## Uso rapido
F9 abre o menu, F8 liga/desliga a camera livre (mude em Settings). Textures: Exportar PNG -> Open Texture Folder ->
edite o PNG -> Reload Texture. Pasta: <jogo>\\FilmStudio\\Textures\\<TXD>\\.
