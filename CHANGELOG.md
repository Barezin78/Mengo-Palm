# Changelog

## v1.1.0 — 24/10/2026
### Build
- Sem mudar a versão do app: `make` também gera `dist/Mengao-v1.1.0.prc`. A versão é lida da linha `VERSION` em `src/mengao.rcp` (um lugar só). O nome interno do banco continua `Mengao` (`build-prc -n Mengao`), então instalar o PRC novo substitui o antigo.
- `make data` também grava `dist/MengaoDados-AAAA-MM-DD.pdb`, com a data do dia em que você gerou os dados. Essas cópias com versão/data não entram no git; vão no release.

### Mudou
- **Tela inicial sem os 4 botões** (Jogos/Tabela/Elenco/Títulos), que repetiam as abas. No lugar, uma **mini tabela do Brasileirão**: o time acima, o Flamengo (destacado) e o time abaixo. Se o Flamengo for o 1º, mostra do 1º ao 3º. São 3 linhas em 320×480 e 2 em 320×320.
- **Toques:** cartão "Próximo jogo" → Jogos; mini tabela → Tabela. O relógio e o resto da tela não fazem nada.
- **Menu Opções** ganhou Jogos (/J), Tabela (/T), Elenco (/E) e Títulos (/L).
- Os botões de rolagem/5-way na tela inicial continuam abrindo Jogos. Nas abas, o botão **Início** volta como antes.
- A linha "Dados de …" e o placar completo dos pênaltis saíram da tela inicial (o placar completo continua no Detalhes do jogo, em Jogos). A data dos dados continua nas abas e em Sobre.

## v1.0.2 — 24/10/2026
### Corrigido
- **Pênaltis cortados no "Último" resultado.** O texto "(pên)" encavalava com a data no rodapé da tela inicial. Agora a linha é medida com a fonte do Palm (`FntCharsWidth`) e encolhe até caber: "pên. 4x3" → "(p)" → só o placar. Se nem o placar couber, o rótulo vira "Últ.:".
  - Em 320×480 o placar dos pênaltis aparece completo na 2ª linha ("Pênaltis: 2x3 · dados …").
  - O placar completo continua no Detalhes do jogo, na aba Jogos.
- **Aba Jogos:** o confronto (ex.: "FLA x CAM") não encosta mais no horário/resultado. O horário é desenhado primeiro e o confronto usa o espaço que sobrar; se precisar, vai em fonte normal.

## v1.0.1 — 24/10/2026
### Corrigido
- **Tela inicial piscando.** O app redesenhava a tela inicial inteira, direto na tela, a cada evento ocioso (`nilEvent`): primeiro o fundo listrado, depois os painéis, depois os textos. Esses passos apareciam como piscadas várias vezes por segundo.
  - Agora tudo é desenhado num buffer fora da tela e copiado pronto de uma vez (`WinCreateOffscreenWindow` + `WinCopyRectangle`).
  - O relógio só redesenha quando o minuto muda. Eventos ociosos extras não fazem nada.
  - `frmUpdateEvent` e `winDisplayChangedEvent` não desenham mais duas vezes (`FrmDrawForm` + desenho). A mudança de tela só redesenha se o tamanho mudou de verdade.
  - Rolar as listas e trocar de aba também usam o buffer, sem piscar.
- O buffer é recriado quando a tela muda entre 320×480 e 320×320 e é liberado ao fechar o app.

## v1.0 — 24/10/2026
- Primeira versão: telas Início (fundo listrado, escudo estilizado, relógio, próximo jogo e último resultado), Jogos, Tabela, Elenco e Títulos.
- Suporte a 320×480 (Tungsten T3, área de Graffiti fechada) e 320×320.
- Dados da ESPN via `MengaoDados.pdb` (`tools/gerar_dados.py`).
