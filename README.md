# FIAP - Faculdade de Informática e Administração Paulista

<p align="center">
  <a href="https://www.fiap.com.br/">
    <img src="assets/img/logo-fiap.jpg" alt="FIAP - Faculdade de Informática e Administração Paulista" border="0" width="40%" height="40%">
  </a>
</p>

## 👥 Grupo 80

## 👨‍🎓 Integrantes

- Amanda Vieira Pires (RM566330)
- Ana Gabriela Soares Santos (RM565235)
- Bianca Nascimento de Santa Cruz Oliveira (RM561390)
- Milena Pereira dos Santos Silva (RM565464)
- Nayana Mehta Miazaki (RM565045)

## 👩‍🏫 Professores

### Tutor(a)

- Lucas Gomes Moreira

### Coordenador(a)

- André Godoi

---

# ❤️ CardioIA – Projeto Integrador

O **CardioIA** é um projeto acadêmico que simula o ecossistema de uma cardiologia moderna, integrando dados clínicos, Machine Learning, Visão Computacional, IoT e agentes inteligentes para triagem, diagnósticos, monitoramento e previsões médicas.

# 📦 Fase 1 – Batimentos de Dados

## 🎯 Visão geral

Nesta **Fase 1 – Batimentos de Dados**, assumimos o papel de cientistas de dados hospitalares: o desafio é levantar, organizar e entender dados cardiológicos que, nas fases seguintes, alimentarão os módulos inteligentes do CardioIA. A entrega contempla três tipos de dados — numéricos, textuais e visuais — com atenção à **governança de dados** e ao **viés**.

## 📋 Objetivos e entregas

- **Parte 1 – Dados numéricos (IoT):** dataset com variáveis clínicas cardíacas (mín. 100 linhas), organizado e documentado.
- **Parte 2 – Dados textuais (NLP):** mínimo dois textos (.txt) sobre doenças cardíacas/saúde cardiovascular, para exploração com NLP.
- **Parte 3 – Dados visuais (VC):** mínimo 100 imagens de exame cardiológico (ex.: ECG, raio-X torácico ou angiograma), para análise com Visão Computacional.

Os conjuntos completos estão disponíveis via **links públicos** (Google Drive, OneDrive ou similar), indicados nas seções abaixo.

---

## 📊 Parte 1 – Dados numéricos (IoT)

**Objetivo:** Dataset para prever **risco de doença cardíaca** (input: idade, pressão, colesterol, sintomas etc. → output: presença/ausência de doença).

### Origem dos dados

- **Abordagem:** híbrida (base real limpa; dados simulados poderão ser agregados em etapa posterior para ampliar o conjunto).
- **Dados reais:** [UCI Machine Learning Repository – Heart Disease Dataset (Cleveland)](https://archive.ics.uci.edu/dataset/45/heart+disease), arquivo processado (14 atributos). Licença de uso acadêmico e pesquisa. Dados anonimizados.
- **Tratamento:** Os dados brutos foram explorados no notebook `notebooks/eda_heart_cleveland.ipynb` e limpos pelo script `scripts/clean_heart_data.py`: substituição de `"?"` em **ca** e **thal** pela moda, criação da coluna **target_binario** (0 = sem doença, 1 = risco). O valor extremo de **colesterol (564 mg/dL)** foi mantido como caso real e documentado (decisão consciente após EDA).
- **Arquivo entregue:** `data/heart_real_cleaned.csv` (303 linhas × 15 colunas, incluindo `target_binario`).

### 🔗 Link para o dataset completo

- O conjunto de dados processado está disponível na pasta [data/heart_real_cleaned.csv](./data/heart_real_cleaned.csv) deste repositório.

**Como reproduzir a limpeza (Parte 1):** na raiz do repositório, com o ambiente ativado (`pip install -r requirements.txt`), executar `python scripts/clean_heart_data.py`. O script lê `data/heart_disease_cleveland_raw.csv` e gera `data/heart_real_cleaned.csv`.

### Variáveis e relevância clínica

| Variável   | Descrição | Relevância para IA em saúde |
|------------|-----------|-----------------------------|
| age        | Idade (anos) | Fator de risco em scores (ex.: Framingham); entrada em modelos de triagem e predição. |
| sex        | Sexo (1=homem, 0=mulher) | Diferenças de risco entre sexos; variável de controle em modelos. |
| cp         | Tipo de dor no peito (1–4) | Sintoma clínico relevante para classificação e triagem. |
| trestbps   | Pressão arterial em repouso (mmHg) | Hipertensão é fator de risco; uso em scores e modelos preditivos. |
| chol       | Colesterol sérico (mg/dL) | Fator de risco cardiovascular; entrada em modelos de risco. |
| fbs        | Glicemia em jejum > 120 (0/1) | Indicador de diabetes; comorbidade em modelos. |
| restecg    | ECG em repouso (0,1,2) | Achado objetivo; suporte a triagem e diagnóstico. |
| thalach    | Frequência cardíaca máxima no teste (bpm) | Capacidade funcional; fortemente associado ao target na EDA. |
| exang      | Angina induzida por exercício (0/1) | Sintoma de isquemia; relevante para classificação. |
| oldpeak    | Depressão do segmento ST | Marcador de isquemia no teste de esforço. |
| slope      | Inclinação do segmento ST (1–3) | Achado de ECG no esforço. |
| ca         | Nº de vasos principais (0–3) | Severidade angiográfica; preditor forte. |
| thal       | Talassemia (3/6/7) | Resultado do teste de esforço. |
| target     | Diagnóstico original (0–4) | 0 = sem doença; 1–4 = graus de doença. |
| target_binario | Risco binário (0/1) | Variável alvo para modelo: 0 = sem risco, 1 = risco. |

**Variáveis mais relevantes para o modelo de risco:** **thalach** (FC máxima), **idade** e **trestbps** (pressão) mostraram tendência clara com o target na EDA; **ca** e **thal** são preditores clínicos fortes. O colesterol isolado teve distribuição similar entre as classes; útil em combinação com as demais.

**Tamanho do dataset:** 303 linhas, 15 variáveis (14 originais + `target_binario`).

---

## 📄 Parte 2 – Dados textuais (NLP)

**Objetivo:** textos (.txt) sobre doenças cardíacas ou saúde cardiovascular, como podem ser usados em NLP e por que isso é relevante para o CardioIA.

**Documentos:** arquivos .txt em `docs/`

### Possíveis aplicações de NLP
- **Extração de sintomas:** os algoritmos de NLP podem identificar sintomas mencionados nos textos, como dor no peito, fadiga, tontura e falta de ar e transformá-las em variáveis estruturadas equivalentes às utilizadas no dataset.
- **Classificação de risco cardiovascular:** os textos podem classificar em categorias como: fatores de risco, sintomas, diagnóstico e tratamento a fim de identificar padrões clínicos.
- **Integração com modelo Machine Learning:** os dados dos textos podem ser convertidos em variáveis numéricas semelhantes ao dataset para alimentar um modelo de ML capaz de prever a presença de doença cardíaca.

### Governança de dados
- **Qualidade e confiabilidade:** textos obtidos por instituições reconhecidadas: Organização Mundial da Saúde e Ministério da Saúde.
- **Privacidade dos dados:** os textos são informativos públicos e portanto não contém dados pessoais e registros clínicos.

### Conexão com Dataset
Os textos coletados neste repositório foram selecionados a partir de fontes institucionais confiáveis e abordam sintomas, fatores de risco e características clínicas de doenças cardiovasculares. Esses conteúdos apresentam termos médicos que correspondem a variáveis presentes no dataset.

Técnicas de **Processamento de Linguagem Natural (NLP)** podem ser utilizadas para extrair essas informações de textos médicos não estruturados e transformá-las em variáveis estruturadas semelhantes às utilizadas no dataset.

| Informação presente nos textos | Variável no dataset Cleveland | Descrição |
|---|---|---|
| idade do paciente | age | Idade em anos |
| sexo | sex | Sexo biológico do paciente |
| dor ou desconforto no peito | cp | Tipo de dor no peito |
| pressão arterial elevada | trestbps | Pressão arterial em repouso |
| colesterol elevado | chol | Nível de colesterol no sangue |
| sintomas durante esforço físico | exang | Angina induzida por exercício |
| frequência cardíaca | thalach | Capacidade funcional; fortemente associado ao target na EDA. |

### Fontes de dados

1. [Organização Mundial da Saúde – Doenças Cardiovasculares](https://www.who.int/news-room/fact-sheets/detail/cardiovascular-diseases-(cvds))

2. [Ministério da Saúde – Infarto Agudo do Miocárdio](https://www.gov.br/saude/pt-br/assuntos/saude-de-a-a-z/i/infarto)

---

## 🖼️ Parte 3 – Dados visuais (Visão computacional)

Este segmento apresenta um conjunto de **140 imagens de Eletrocardiograma (ECG)** utilizadas para fins educacionais e de estudo na aplicação de técnicas de **Visão Computacional** e **Inteligência Artificial** na área da saúde.

O objetivo deste dataset é demonstrar como algoritmos de análise de imagens podem identificar padrões presentes em exames cardiológicos e auxiliar no desenvolvimento de sistemas inteligentes para análise médica.

---

### 📂 Acesso ao Dataset

As imagens utilizadas neste projeto estão hospedadas em um repositório externo para facilitar o acesso e download.

**Link para acesso ao dataset:**  
*https://drive.google.com/drive/folders/1Db4h6M2dsE2K_kz8VsePKqeUYzokrxdY*

Todas as imagens estão organizadas em uma única pasta contendo registros de ECG de diferentes condições cardíacas.

---

### 📊 Descrição do Dataset

O conjunto de dados contém **140 imagens de ECG**, divididas igualmente em quatro categorias que representam diferentes condições cardíacas. Cada categoria foi identificada por uma sigla para facilitar a organização e análise dos dados.

| Sigla | Categoria | Descrição | Quantidade |
|------|------|------|------|
| **MI** | Myocardial Infarction | ECG de pacientes diagnosticados com infarto do miocárdio | 35 |
| **HB** | Abnormal Heartbeat | ECG de pacientes com batimentos cardíacos anormais ou arritmias | 35 |
| **NO** | Normal | ECG de indivíduos com atividade cardíaca considerada normal | 35 |
| **PMI** | Previous Myocardial Infarction | ECG de pacientes com histórico prévio de infarto do miocárdio | 35 |

**Total de imagens:** 140

Essa organização permite que os dados sejam utilizados em experimentos de classificação de imagens e análise automática de padrões cardíacos.

---

### 🤖 Aplicação de Visão Computacional

As imagens de **Eletrocardiograma (ECG)** presentes neste dataset podem ser analisadas por algoritmos de **Visão Computacional** com o objetivo de identificar padrões relacionados à atividade elétrica do coração. O ECG representa graficamente os sinais elétricos cardíacos por meio de ondas características (P, QRS e T), que refletem diferentes etapas do ciclo cardíaco. Alterações na forma, amplitude ou intervalo dessas ondas podem indicar diversas condições cardiovasculares.

A partir dessas imagens, técnicas de processamento e análise de imagens podem ser aplicadas para extrair informações relevantes e auxiliar na identificação de possíveis anomalias cardíacas. Entre as principais abordagens utilizadas estão:

### Detecção de bordas

Algoritmos de detecção de bordas podem identificar os contornos das ondas presentes no traçado do ECG. Essa técnica permite destacar as transições entre diferentes regiões da imagem, facilitando a identificação das ondas características do exame. A partir disso, é possível analisar a morfologia das ondas e detectar variações associadas a determinadas condições cardíacas.

### Extração de características

Após a identificação das principais estruturas do ECG, algoritmos de **extração de características** podem ser utilizados para capturar informações relevantes do sinal. Essas características podem incluir:

- amplitude das ondas
- duração dos intervalos entre batimentos
- variações no formato do complexo QRS
- padrões de repetição no ritmo cardíaco

Esses atributos podem ser utilizados como entrada para modelos de aprendizado de máquina que buscam diferenciar sinais cardíacos normais de sinais associados a doenças cardiovasculares.

### Reconhecimento de padrões

Outra aplicação importante da Visão Computacional é o **reconhecimento automático de padrões**. Por meio da análise de múltiplos exemplos de ECG, algoritmos podem aprender a identificar padrões característicos associados a determinadas condições clínicas, como alterações no ritmo cardíaco ou evidências de infarto do miocárdio.

Esse tipo de abordagem permite que sistemas computacionais identifiquem semelhanças entre novos exames e padrões previamente aprendidos durante o treinamento do modelo.

### Classificação com aprendizado profundo

Modelos baseados em **Redes Neurais Convolucionais (CNN)** podem ser utilizados para realizar a classificação automática das imagens do dataset nas diferentes categorias (MI, HB, NO e PMI). Essas redes são especialmente eficientes na análise de imagens, pois conseguem aprender automaticamente padrões visuais complexos sem a necessidade de extração manual de características.

Durante o processo de treinamento, o modelo aprende a identificar diferenças sutis entre ECGs normais e ECGs associados a condições cardíacas específicas, permitindo posteriormente classificar novos exames de forma automatizada.

---

### 🧠 Importância para Projetos de Inteligência Artificial na Saúde

A aplicação de técnicas de **Inteligência Artificial** e **Visão Computacional** na análise de exames cardiológicos possui grande potencial para transformar a forma como doenças cardiovasculares são diagnosticadas e monitoradas.

Doenças cardíacas estão entre as principais causas de morte no mundo, e a interpretação de exames como o ECG exige conhecimento especializado e análise cuidadosa por parte de profissionais de saúde. Nesse contexto, sistemas baseados em IA podem atuar como ferramentas de apoio ao diagnóstico, auxiliando médicos na identificação de padrões anormais nos exames.

Entre os principais benefícios do uso dessas tecnologias estão:

- **Detecção precoce de doenças cardiovasculares**, permitindo intervenções médicas mais rápidas  
- **Automatização da análise de exames**, reduzindo o tempo necessário para avaliação clínica  
- **Apoio à tomada de decisão médica**, oferecendo uma segunda análise baseada em dados  
- **Redução de erros humanos**, principalmente em ambientes com grande volume de exames  
- **Possibilidade de triagem automática**, identificando rapidamente exames que requerem atenção médica imediata  

Além disso, o desenvolvimento de modelos de IA treinados com dados como os presentes neste dataset pode contribuir para a criação de sistemas de **diagnóstico assistido por computador (Computer-Aided Diagnosis – CAD)**. Esses sistemas são cada vez mais utilizados em diversas áreas da medicina para auxiliar na interpretação de exames médicos.

Portanto, a análise de imagens de ECG por meio de técnicas de Visão Computacional representa uma abordagem promissora para o desenvolvimento de soluções tecnológicas capazes de melhorar a eficiência, a precisão e a acessibilidade do diagnóstico de doenças cardiovasculares.

---

### 📚 Fonte dos Dados

As imagens utilizadas neste projeto foram obtidas a partir do dataset público **ECG Images Dataset of Cardiac Patients**, disponível na plataforma **Mendeley Data**.

O dataset foi desenvolvido para apoiar pesquisas relacionadas à análise de doenças cardiovasculares por meio de técnicas de aprendizado de máquina e processamento de sinais cardíacos.

**Fonte original do dataset:**  
https://data.mendeley.com/datasets/gwbz3fsgp8/2

**Autores do dataset:**
- Ali Haider Khan  
- Muzammil Hussain  

**Licença:** Creative Commons Attribution 4.0 (CC BY 4.0)

---

## ⚖️ Governança de dados e viés

- **Rastreabilidade:** Parte 1: origem UCI/Cleveland citada; limpeza documentada no notebook de EDA e no script `clean_heart_data.py`. Partes 2 e 3: fontes e links descritos nas seções correspondentes.
- **Viés:** O dataset Cleveland refere-se a um único centro e uma população específica (EUA, período do estudo); resultados de modelos treinados nessa base podem não generalizar diretamente para outros contextos. O valor 564 (colesterol) foi mantido por decisão explícita após análise (caso extremo real).
- **Uso ético:** Dados anonimizados; licença e citação da fonte (UCI) respeitadas.

---

## 🛠 Tecnologias utilizadas (Fase 1)

- **Versionamento:** Git / GitHub
- **Parte 1 – Dados numéricos:** Python 3, Pandas, Jupyter (EDA em notebook; scripts `load_dataset.py` e `clean_heart_data.py`)
- **Armazenamento dos conjuntos completos:** Links públicos (Google Drive / OneDrive) nas seções correspondentes

---

# 🌐 Fase 3 – IoT na Saúde: Monitoramento Contínuo

## 🎯 Visão geral

Nesta **Fase 3**, o projeto CardioIA avança para a simulação de um sistema vestível de monitoramento cardíaco contínuo, integrando conceitos de **Edge Computing**, **protocolo MQTT**, **armazenamento local** e **dashboards interativos**.

---

## 🔌 Parte 1 – Armazenamento e Processamento Local (Edge Computing)

### Descrição

Protótipo desenvolvido no **Wokwi** com **ESP32**, simulando um dispositivo vestível capaz de capturar sinais vitais, armazenar dados localmente durante períodos offline e sincronizá-los com a nuvem ao reconectar.

### Sensores utilizados

| Componente | Descrição | Pino ESP32 |
|---|---|---|
| DHT22 | Temperatura (0–50°C) e umidade (0–100%) | GPIO4 |
| Push Button | Simulador de batimentos cardíacos (BPM) | GPIO15 |

### Funcionalidades

- Leitura periódica de temperatura, umidade e BPM a cada 2 segundos
- Buffer circular em RAM com capacidade para 100 amostras (~3min20s offline)
- Estratégia FIFO para buffer cheio — preserva dados mais recentes
- Sincronização automática ao reconectar (simulada por variável booleana)
- Alertas clínicos: temperatura > 38°C, umidade > 90%, BPM > 120
- Serialização dos dados em formato JSON compatível com MQTT

### 🔗 Links

- **Simulação no Wokwi:** https://wokwi.com/projects/463031226583237633
- **Código:** [`phase03-cardioia-iot/parte1-edge-computing/sketch.ino`](./phase03-cardioia-iot/parte1-edge-computing/sketch.ino)
- **Diagrama:** [`phase03-cardioia-iot/parte1-edge-computing/diagram.json`](./phase03-cardioia-iot/parte1-edge-computing/diagram.json)
- **Relatório:** [`phase03-cardioia-iot/parte1-edge-computing/relatorio-parte1.pdf`](./phase03-cardioia-iot/parte1-edge-computing/relatorio-parte1.pdf)

---

## 📡 Parte 2 – Transmissão para Nuvem e Visualização (em desenvolvimento)

> Conteúdo a ser adicionado após conclusão da Parte 2.

---

## 📁 Estrutura atualizada do repositório

```
chap01-phase01-cardioia-dataset-foundation/
├── README.md
├── assets/img/
├── data/                        # Fase 1 – Parte 1: dados numéricos
│   ├── heart_disease_cleveland_raw.csv
│   └── heart_real_cleaned.csv
├── docs/                        # Fase 1 – Parte 2: textos NLP
├── notebooks/                   # Fase 1 – EDA
│   └── eda_heart_cleveland.ipynb
├── scripts/                     # Fase 1 – scripts de limpeza
│   ├── load_dataset.py
│   └── clean_heart_data.py
├── requirements.txt
├── .gitignore
└── phase03-cardioia-iot/        # Fase 3 – IoT na Saúde
    ├── parte1-edge-computing/
    │   ├── sketch.ino
    │   ├── diagram.json
    │   └── relatorio-parte1.pdf
    └── parte2-mqtt-dashboard/   # em desenvolvimento
```

### 🛠 Tecnologias utilizadas (Fase 3)

- **Hardware simulado:** ESP32 (Wokwi)
- **Sensores:** DHT22, Push Button
- **Linguagem:** C++ (Arduino)
- **Protocolos:** MQTT (Parte 2)
- **Visualização:** Node-RED, Grafana (Parte 2)
- **Conceitos:** Edge Computing, Fog Computing, Cloud Computing
