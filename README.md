## Jogo da Soma

## Descrição
O **Jogo da Soma** é um jogo educativo desenvolvido para a placa **BitDogLab**, baseada no microcontrolador **RP2040** da Raspberry Pi Pico W. O objetivo do jogo é testar a habilidade do jogador em identificar se uma soma apresentada está correta ou não, utilizando os botões para responder. O jogo fornece feedback visual através de um display OLED e auditivo com um buzzer em caso de erro.

## Materiais Necessários
- Placa **BitDogLab**
- Fonte de alimentação USB
- Display **OLED SSD1306**
- Buzzer piezoelétrico
- Dois botões (A e B)

## Esquemático de Conexões com a BitDogLab

| Pino do Componente | Pino da BitDogLab |
|---------------------|-------------------|
| Botão A           | GP5               |
| Botão B           | GP6               |
| Buzzer            | GP21              |
| I2C SDA (OLED)    | GP14              |
| I2C SCL (OLED)    | GP15              |

## Configuração do CMakeLists.txt
```cmake
# Adiciona o arquivo-fonte correto
add_executable(JogoSoma JogoSoma.c inc/ssd1306.c)

# Adiciona bibliotecas necessárias
target_link_libraries(JogoSoma pico_stdlib hardware_timer hardware_pwm hardware_clocks hardware_gpio hardware_i2c)
```

## Explicação do Código
1. **Inicialização:** O código inicia configurando os pinos GPIO, inicializando a comunicação I2C para o display OLED e configurando o PWM para o buzzer.
2. **Exibição de Mensagens:** O display OLED é utilizado para mostrar as operações matemáticas e mensagens de feedback.
3. **Geração de Perguntas:** O jogo gera uma soma aleatória e decide se a resposta será correta ou errada.
4. **Interação do Jogador:** Os botões A e B são utilizados para responder se a soma está correta ou não.
5. **Feedback:** Se a resposta for correta, o jogador ganha pontos. Se for errada, um som é emitido pelo buzzer e o jogo termina.
6. **Reinicialização:** O jogador pode reiniciar o jogo pressionando o botão A.

## Testando o Jogo
1. Conecte a **BitDogLab** ao computador via **USB**.
2. Compile e carregue o código utilizando o **SDK do Raspberry Pi Pico**.
3. Pressione o **botão A** para iniciar o jogo.
4. Leia a soma exibida no **display OLED**.
5. Pressione **A** se a soma estiver correta ou **B** se estiver errada.
6. O jogo exibirá feedback e continuará até o jogador errar.

## Conclusão
Este projeto demonstra como desenvolver um jogo educativo simples utilizando a placa **BitDogLab** e o microcontrolador **RP2040**. A integração de componentes como display OLED, botões e buzzer cria uma experiência interativa, aplicável a outras aplicações educacionais e embarcadas.

