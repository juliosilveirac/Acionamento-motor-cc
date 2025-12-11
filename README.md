# Projeto e Implementação de Acionamento para Motores CC

Este repositório contém os códigos-fonte (**Firmware** e **Software Supervisório**) desenvolvidos como parte do Trabalho de Conclusão de Curso (TCC).

---

## 📝 Sobre o Projeto

O objetivo deste trabalho foi desenvolver um protótipo didático para o acionamento e instrumentação de motores de corrente contínua (CC) de excitação independente. O sistema permite o controle independente dos retificadores da armadura e do campo, além de realizar a leitura de sensores de tensão, corrente e velocidade.

**Diferencial:** O projeto utiliza uma arquitetura de baixo custo baseada no microcontrolador **ESP32**, operando com sistema de tarefas em *Dual Core* para garantir a precisão dos disparos dos tiristores em sincronia com a rede elétrica.

---

## 🚀 Funcionalidades

* **Acionamento Controlado:** Controle de fase (ângulo de disparo) para duas pontes retificadoras trifásicas (3 pulsos) independentes (Armadura e Campo).
* **Instrumentação Digital:** Leitura de Tensão de Armadura/Campo, Corrente de Armadura/Campo e Velocidade (Tacogerador).
* **Monitoramento em Tempo Real:** Interface supervisória desenvolvida em **LabVIEW** para visualização de gráficos e parametrização.
* **Comunicação:** Protocolo Modbus RTU via Serial.

---

## 🛠️ Hardware e Pinagem

O firmware foi desenvolvido para o **ESP32 DevKitC**. Abaixo está o mapeamento dos pinos (GPIO) utilizados conforme o projeto:

### 1. Entradas Analógicas
| Função | Pino (GPIO) | Descrição |
| :--- | :---: | :--- |
| **Tensão Armadura** | 36 | Leitura do divisor de tensão |
| **Corrente Armadura** | 39 | Leitura do Shunt/Amplificador |
| **Tensão Campo** | 34 | Leitura do divisor de tensão |
| **Corrente Campo** | 35 | Leitura do Shunt/Amplificador |
| **Tacogerador (RPM)** | 32 | Leitura de velocidade |

### 2. Sincronismo
| Função | Pino (GPIO) | Descrição |
| :--- | :---: | :--- |
| **Sincronismo Fase R** | 33 | Detector de passagem por zero |
| **Sincronismo Fase S** | 25 | Detector de passagem por zero |
| **Sincronismo Fase T** | 26 | Detector de passagem por zero |

### 3. Saídas de Disparo dos Tiristores
| Função | Pino (GPIO) | Descrição |
| :--- | :---: | :--- |
| **Armadura Fase R** | 19 | Pulso de disparo |
| **Armadura Fase S** | 18 | Pulso de disparo |
| **Armadura Fase T** | 5 | Pulso de disparo |
| **Campo Fase R** | 17 | Pulso de disparo |
| **Campo Fase S** | 16 | Pulso de disparo |
| **Campo Fase T** | 4 | Pulso de disparo |

---

## 📄 Licença

Este projeto foi desenvolvido para fins educacionais e acadêmicos. Sinta-se à vontade para utilizar e modificar o código, citando a autoria original.
