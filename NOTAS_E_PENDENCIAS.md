# Notas e pendencias (para resolver quando for testar)

## Historico de erros ja encontrados
1. Upload de pastas no GitHub nao funciona pelo seletor -> projeto foi "achatado" (todos os arquivos na raiz, exceto .github/workflows/build.yml).
2. Workflow: "No Premake script (premake5.lua) found" -> o script fica em tools/premake/premake5.lua. Corrigido para procura-lo.
3. Workflow: "msbuild nao reconhecido" -> adicionado microsoft/setup-msbuild.
4. Workflow: "multiprocessorcompile (nil value)" -> premake beta2 antigo. Workflow agora tenta o premake5.exe do plugin-sdk e depois beta7/6/4. NAO CONFIRMADO ainda.
5. F9 nao fazia nada simplesmente porque nao havia FilmStudio.asi compilado.

## Pontos de MAIOR risco (verificar primeiro ao testar / compilar)
- Compilacao: nomes de classes/campos do plugin-sdk escritos de memoria (podem dar erro de build):
  CTaskSimpleStandStill, CTaskComplexWanderStandard, CTaskSimpleRunNamedAnim (parametros),
  CTaskManager::m_aPrimaryTasks, CStreaming::ms_aInfoForModel[].m_nLoadState,
  CPopulation::IsFemale, CPed::m_nCreatedBy, RsGlobal.maximumWidth, CAnimManager::AddAnimBlockRef.
- Hook D3D9 (Hooks.cpp): se houver Windowed Mode / Widescreen Fix, o EndScene pode nao ser chamado. Ver FilmStudio.log:
  "Hooks instalados" e "ImGui inicializado" devem aparecer.
- Mouse: GTA recentraliza o cursor; bloqueamos SetCursorPos com menu aberto. Se o cursor nao se mover, e aqui.
- Flags de animacao em AnimationManager.cpp sao numericas (0x1 STARTED, 0x2 LOOPED, 0x20 FREEZE_TRANSLATION).
  Se a animacao nao tocar / nao loopar, conferir esses valores.
- IFP_BASE = 25575 (id do primeiro bloco IFP no streaming, 1.0 US).
- Mover com o mouse: usa projecao inversa iterativa (Newton) sobre o plano Z do NPC; Z ajusta com a roda.
- Modo Static: nao congela; trava X/Y a cada frame e mantem StandStill quando nao ha tarefa/animacao.

## Etapa 5 (Textures) - pontos de risco
- Pasta: FilmStudio\\Textures\\<TXD>\\<textura>.png (mesma ideia do Texture Swapper: uma pasta por TXD).
- TXD lido direto do disco (modloader\\ primeiro, depois models\\gta3.img e player.img) e decodificado por conta propria
  (TxdFile.cpp: DXT1/3/5, 8888, 888, 565, 1555, 4444, PAL8). Offsets do cabecalho D3D8/D3D9 escritos de memoria: se as
  imagens exportadas saírem trocadas/erradas, o problema esta ali (FilmStudio.log mostra de onde o TXD foi lido).
- Reload troca o raster da RwTexture ja existente (afeta todos os NPCs com o mesmo TXD). Mantemos AddRef no TXD ate Restaurar.
- CJ: so o modelo base (TXD "player"). Roupas de CJ (player.img + RebuildPlayer) NAO suportadas ainda.
- Campos plugin-sdk a conferir: CTxdStore::ms_pTxdPool->GetAt()->m_pRwDictionary, CBaseModelInfo::m_nTxdIndex, RwRaster::stride.

## Etapa 6 (Camera) - pontos de risco
- Usa SetCamPositionForFixedMode + TakeControlNoEntity todo frame (como os opcodes 015F/0160) e RestoreWithJumpCut ao desligar.
- FOV via TheCamera.m_aCams[m_nActiveCam].m_fFOV (pode ser sobrescrito pelo jogo/Widescreen Fix).
- Mouse da camera vem de CPad::NewMouseControllerState.X/Y (sensibilidade ajustavel). Sem roll.
- CHud::m_Wants_To_Draw_Hud para esconder HUD.

## Estado das etapas
1 Fundacao: escrita | 2 NPC Manager: escrita | 3 Position+Behavior: escrita | 4 Animation: escrita
5 Textures (NPCs + base do CJ): escrita | 6 Camera: escrita | 7 Integracao/testes: pendente. NADA compilado/testado ainda.
