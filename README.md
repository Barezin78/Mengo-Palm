# Mengão para Palm OS 🔴⚫ (v1.1.1)

App de fã do Flamengo para o **Palm Tungsten T3** (Palm OS 5.2). Ele usa a tela inteira de 320×480 com o slider aberto e também funciona em 320×320.

- **Início:** fundo listrado rubro-negro, escudo estilizado "CRF" (não é o escudo oficial) e relógio grande com a data. Tem o cartão **Próximo jogo** com contagem regressiva, o **último resultado** e uma **mini tabela do Brasileirão** com o Flamengo destacado e os vizinhos (3 linhas em 320×480, 2 em 320×320). Toque no cartão para abrir **Jogos** e na mini tabela para abrir **Tabela**.
- **Jogos:** todos os jogos da temporada, com resultado (verde = vitória, amarelo = empate, vermelho = derrota, "p" = pênaltis) ou horário dos próximos. Toque num jogo para selecionar. Tocar de novo ou usar **Detalhes** mostra data, estádio e placar.
- **Tabela:** classificação do Brasileirão, com o Flamengo destacado.
- **Elenco:** número, posição e nome dos jogadores.
- **Títulos:** os principais títulos, conferidos em flamengo.com.br e no ge (out/2026).
- **Menu (Opções):** Início, Jogos, Tabela, Elenco, Títulos e Sobre o Mengão. Nas abas, o botão **Início** volta para a tela inicial.
- Na tela inicial, os botões de rolagem do Palm (e o 5-way para cima/baixo) abrem Jogos. Nas abas, eles rolam as listas. Também dá para tocar nas setas da barra de rolagem.

O Palm não tem internet, então os dados vêm num arquivo separado, o **`MengaoDados.pdb`**. Sem ele, o app abre normalmente e mostra "Importe MengaoDados.pdb".

## Arquivos
| Arquivo | O que é |
|---|---|
| `dist/Mengao.prc` | o app (é este que você instala no Palm) |
| `dist/MengaoDados.pdb` | os dados (jogos, tabela, elenco), gerados da ESPN |
| `dist/Mengao-vX.Y.Z.prc` | o mesmo app, com a versão no nome. O `make` gera; fica de fora do git e vai no release |
| `dist/MengaoDados-AAAA-MM-DD.pdb` | cópia datada dos dados. O `make data` gera; também fica de fora do git |
| `preview/` | prévias das telas (simulação no computador, veja "Testes") |
| `src/` | código-fonte (C + recursos pilrc) |
| `CHANGELOG.md` | o que mudou em cada versão |
| `tools/gerar_dados.py` | gera um `MengaoDados.pdb` novo com dados atuais |

## Instalar no CloudpilotEmu (iPhone/iPad)
1. Passe `Mengao.prc` e `MengaoDados.pdb` para o iPhone, por AirDrop, Arquivos/iCloud Drive ou pelo e-mail.
2. Abra o CloudpilotEmu com o seu Palm virtual ligado.
3. Toque no botão de **instalar arquivo** (ícone de download/"+" na barra) e escolha o `Mengao.prc`. Repita com o `MengaoDados.pdb`. Também dá pra abrir o arquivo no app Arquivos e usar "Compartilhar → CloudpilotEmu".
4. O ícone **Mengão** (escudo rubro-negro) aparece no Launcher.

> Se o seu dispositivo virtual tiver tela baixa resolução (160×160, como o m515), o app avisa que precisa de tela hi-res. Use um Palm OS 5 hi-res no emulador.

## Instalar no Tungsten T3 de verdade
**Pelo cartão SD (mais fácil hoje):**
1. Copie os dois arquivos para o cartão, na pasta `/PALM/Launcher/`.
2. No T3, abra o Launcher e escolha a categoria do cartão.
3. Menu → **Copiar** → copie **Mengão** e **MengaoDados** para o **Palm (Handheld)**.
   ⚠️ O `MengaoDados.pdb` precisa estar **na memória do Palm**, não só no cartão. O app só lê dados da memória interna.

**Por HotSync:** com o Palm Desktop/HotSync Manager (Windows), abra a **Ferramenta de Instalação** (Install Tool), adicione os dois arquivos e faça um HotSync.

## Atualizar os dados
No computador (precisa de Python 3 e internet):
```
python3 tools/gerar_dados.py dist/MengaoDados.pdb
```
O `make data` faz a mesma coisa e ainda guarda uma cópia `dist/MengaoDados-AAAA-MM-DD.pdb`. Depois instale o `MengaoDados.pdb` novo do mesmo jeito. Ele substitui o antigo, mas convém fechar o app antes. Os horários ficam no horário de Brasília. O app usa o relógio do Palm, então deixe o Palm no fuso de Brasília.

## Compilar
Precisa de Linux com [prc-tools-remix](https://github.com/jichu4n/prc-tools-remix) e [pilrc](https://github.com/jichu4n/pilrc).

O Palm OS SDK **não vem neste repositório** (é de terceiros, uns 111 MB, e o `sdk/` está no `.gitignore`). O `make` procura os headers em `sdk/sdk-5r3/include`. Pegue o arquivo [jichu4n/palm-os-sdk](https://github.com/jichu4n/palm-os-sdk) e deixe a pasta `sdk-5r3` nesse caminho:

```
git clone --depth 1 https://github.com/jichu4n/palm-os-sdk.git sdk
```

O clone já cria `sdk/sdk-5r3/` (e as outras versões do SDK, que o Mengão não usa). Se o arquivo já estiver em outro lugar:

```
mkdir -p sdk
ln -s /caminho/para/palm-os-sdk/sdk-5r3 sdk/sdk-5r3
```

Aí é só:

```
make          # gera dist/Mengao.prc e dist/Mengao-v1.1.1.prc
make data     # gera dist/MengaoDados.pdb e dist/MengaoDados-AAAA-MM-DD.pdb
```

A versão do arquivo (`Mengao-v1.1.1.prc`) sai da linha `VERSION` em `src/mengao.rcp` — é o único lugar. O nome interno do app continua **Mengao** (`build-prc -n Mengao`), então instalar o PRC novo substitui a versão antiga no Palm. A cópia do PDB com a data é só pra guardar/release; o app continua lendo o banco `MengaoDados`.

O código fica em UTF-8 e o Makefile converte para CP1252 (a codificação do Palm) antes de compilar.

## Detalhes técnicos
- Creator ID `MNGO`. O app é `Mengao` (tipo `appl`) e os dados são `MengaoDados` (tipo `DATA`).
- Sem piscada: cada tela é desenhada primeiro num buffer fora da tela (`WinCreateOffscreenWindow`, formato nativo) e copiada pronta com `WinCopyRectangle`. Na tela inicial o relógio só redesenha quando o minuto muda.
- Tudo é desenhado em coordenadas nativas (densidade dupla) com `WinSetCoordinateSystem`. O relógio usa dígitos desenhados (estilo 7 segmentos). Os textos usam as fontes do próprio Palm.
- 320×480: o app usa a política DIA customizada (`FrmSetDIAPolicyAttr`, `WinSetConstraintsSize`), fecha a área de Graffiti (`PINSetInputAreaState`) e se redesenha em `winDisplayChangedEvent`. Se a área for reaberta, cai no layout de 320×320.
- Formato do PDB: veja o cabeçalho de `tools/gerar_dados.py`.

## Testes (o que foi e o que não foi testado)
- ✅ Compila sem erros nem avisos do código do app com prc-tools-remix 2.3.6 + SDK 5r3. A estrutura do PRC e do PDB foi conferida (recursos `code`, `data`, `tFRM`, `MBAR`, `Talt`, `tAIB`, `tAIN`, `tver`; PDB com 4 registros).
- ✅ O mesmo código C foi compilado no Linux com uma "imitação" das funções do Palm, para gerar as prévias em `preview/` com os dados reais. Também rodou com AddressSanitizer em todas as telas, com rolagem, nos dois tamanhos de tela e sem dados, sem erros.
- ❌ **Não foi testado num Palm/emulador de verdade**: não tenho a ROM do T3 aqui. As fontes reais do Palm são um pouco diferentes das usadas nas prévias. A troca 320×480 ↔ 320×320 e os botões físicos só dá pra confirmar no aparelho/CloudpilotEmu.

## Limitações conhecidas (v1.1.1)
- Sem internet/ponte: os dados só mudam quando você instala um `MengaoDados.pdb` novo.
- O relógio atualiza a cada minuto só na tela inicial.
- Em 320×320 (área de Graffiti aberta) a tela inicial esconde o nome do estádio e mostra só 2 linhas da mini tabela.
- A data de atualização dos dados aparece nas abas Jogos e Elenco e em Sobre (não mais na tela inicial).
- Sem preferências. A lista de títulos é fixa no app.
