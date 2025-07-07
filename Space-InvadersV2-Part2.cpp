#include <iostream>
#include <windows.h>
#include <conio.h>
#include <chrono>
#include <thread>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>

#define ALTURA 25
#define LARGURA 35

using namespace std;

struct Ponto{
    int x, y;
};

struct Personagem{
    Ponto coordenadasP; /// x = mov. -esquerda/+direita,  y = mov. -cima/+baixo
    int vida, score;
    string nome;
};

struct Inimigos{
    Ponto coordenadasI;
    bool vivo;
    int vida, tipo;
};

struct Disparo {
    Ponto coordenadasD;
    int direcao; // 1 para disparo inimigo (pra baixo), -1 para disparo jogador (pra cima)
    bool ativo;  // se disparo está ativo na tela
};

struct Efeito {
    int x, y;
    int duracao; // quantos ticks vai ficar na tela
};

vector<Efeito> efeitos;
vector<Inimigos> inimigos;
vector<Disparo> disparos;

bool poderes[5] = {false}; /// VidaExtra / +velocidade / +tiros / +velDeDisparo / +Pontos / Freeze
int tickGlobal = 0;

// Ticks de ação (controle de velocidade)
int TICK_MOV_JOGADOR              = 2;   // Delay entre movimentos do jogador
const int TICK_INTERVALO_DISPARO  = 5;   // Delay entre tiros do jogador
const int TICK_DISPAROS           = 1;   // Velocidade com que os tiros se movem
const int TICK_MOV_INIMIGOS_BASE  = 15;  // Base da velocidade dos inimigos (ajustada dinamicamente)

// Últimos ticks de ação
int tickUltimoMovJogador      = 0;
int tickUltimoDisparoJogador  = -TICK_INTERVALO_DISPARO;
int tickUltimoMovInimigos     = 0;

int tickMovInimigos = TICK_MOV_INIMIGOS_BASE;   // Velocidade atual dos inimigos (pode ser ajustada dinamicamente)
int direcaoInimigos = 1;                        // Direção dos inimigos: 1 = direita, -1 = esquerda
bool teclaEspacoAnterior = false;               // Para detecção de toque de tecla (ex: espaço)
bool podeTocarSom = true;


void tocarBeep(int timbre) {
    if (podeTocarSom) {
        podeTocarSom = false;
        std::thread([timbre](){
            Beep(timbre, 200);
            podeTocarSom = true;
        }).detach();
    }
}

void mudarCor(int cor) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, cor);
}

void corInf(HANDLE hConsole, int Cursor, int numeroCursor){
    if(Cursor==numeroCursor){
        mudarCor(3); // Ciano
    }else{
        mudarCor(7); // branco
    }
}

void posicao(int x, int y){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD cordenada;
    cordenada.X = x;
    cordenada.Y = y;
    SetConsoleCursorPosition(hConsole, cordenada);
}

void limparTela(int x, int y, int largura, int altura) {
    for (int i = 0; i < altura; i++) {
        posicao(x, y + i);
        for (int j = 0; j < largura; j++) {
            cout << " ";
        }
    }
}

void textoCentralizado(const string& texto, int cor = 7) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);

    int espacos = (csbi.srWindow.Right - texto.length()) / 2;
    SetConsoleTextAttribute(hConsole, cor);
    cout << string(espacos, ' ') << texto << endl;
    SetConsoleTextAttribute(hConsole, 7); // Reset para branco
}

void textoCentralizado(const string& texto, const string& corHex) {
    int cor;
    try {
        cor = stoi(corHex, nullptr, 16); // Converte hex string para int
    } catch (...) {
        cor = 7; // Valor padrão em caso de erro
    }
    textoCentralizado(texto, cor); // Chama a versão numérica
}

void dispararProjetil(const Personagem& personagem, int nave) {

    bool disparoAtivo = false;
    int offsets[] = { -1, 0, 1 };

    if (tickGlobal - tickUltimoDisparoJogador < TICK_INTERVALO_DISPARO) return; // intervalo de 10 ticks
    tickUltimoDisparoJogador = tickGlobal;

    for (const auto& d : disparos) {
        if (d.ativo && d.direcao == -1) {
            disparoAtivo = true; // já tem disparo do jogador ativo
        }
    }

    if (disparoAtivo && poderes[3] == false) return; // se ja tiver disparo ativo E o poder de atirar rapido estiver desativado, somente um disparo por vez permitido

    int x = personagem.coordenadasP.x;
    int y = personagem.coordenadasP.y - 1;

    switch (nave) {
        case 1: // Nave comum
            disparos.push_back({x, y, -1, true }); /// coordenadas x, coordenadas y -1, direcao disparo, ativo = true
            break;

        case 2: // Tiro duplo
            if (x - 2 >= 0 && x + 1 < LARGURA -1){
                disparos.push_back({ x - 1, y, -1, true });
                disparos.push_back({ x + 1, y, -1, true });
            }else if (x - 2 >= 0){
                disparos.push_back({ x - 1, y, -1, true });
                disparos.push_back({ x,     y, -1, true });
            } else /*(x + 1 < LARGURA -1)*/{
                disparos.push_back({ x + 1, y, -1, true });
                disparos.push_back({ x,     y, -1, true });
            }
            break;

        case 3: // Disparo triplo
            disparos.push_back({ x,     y, -1, true });
            if (x - 2 >= 0){
                disparos.push_back({ x - 1, y, -1, true });
            }
            if (x + 1 < LARGURA -1){
                disparos.push_back({ x + 1, y, -1, true });
            }
            break;
    }
}

void movimentacaoPersonagem(char tecla, Personagem &personagem, int mapa[ALTURA][LARGURA]) {
    int newX = personagem.coordenadasP.x;

    if (tecla == 'A') newX--;
    else if (tecla == 'D') newX++;

    if (newX >= 0 && newX < LARGURA) {
        if (mapa[personagem.coordenadasP.y][newX] != 1) {
            personagem.coordenadasP.x = newX;
        }
    }
}

void ordenarRankings(Personagem scores[], int inicio, int fim) {
    if (inicio >= fim) return;

    int temp = scores[fim].score;
    int i = inicio - 1;

    for (int j = inicio; j < fim; j++) {
        if (scores[j].score >= temp) {
            i++;
            swap(scores[i], scores[j]);
        }
    }
    swap(scores[i + 1], scores[fim]);

    ordenarRankings(scores, inicio, i);
    ordenarRankings(scores, i + 2, fim);
}

void exibirTop15() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    const int MAX = 100;
    Personagem scores[MAX];
    int contador = 0;

    ifstream arquivo("pontuacao.txt");
    while (arquivo >> scores[contador].nome >> scores[contador].score && contador < MAX) {
        contador++;
    }
    arquivo.close();

    ordenarRankings(scores, 0, contador - 1);

    system("cls");

    textoCentralizado("=== TOP 15 ===", 5);

    cout << endl << endl;

    for (int i = 0; i < min(15, contador); i++) {
        string linha = to_string(i+1) + ". " + scores[i].nome + " - " + to_string(scores[i].score);
        textoCentralizado(linha);
    }

    posicao(0, 29);

    system("pause");
    system("cls");
}

void informacoesMenu(){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    int cursorAnterior = 0;
    int cursorInfMenuPrincipal=0, cursorInfMenuPrincipalNew;
    char teclaInfMenuPrincipal;
    bool finalizarInfMenuPrincipal = false;

    while(finalizarInfMenuPrincipal==false){
        mudarCor(7); // branco
        posicao (0, 4);
        for(int i=0; i<120; i++){ //imprimir barra inferior
            if(i!=32 && i!=89){
                cout << char(205);
            }else{
                cout << char(202);
            }
        }
        corInf(hConsole, cursorInfMenuPrincipal, 0);
        posicao (13, 2); cout << "Creditos";
        mudarCor(7); // branco

        for(int i=0; i<4; i++){ // imprime barra lateral
            posicao(32, i); cout << char(186);
        }

        corInf(hConsole, cursorInfMenuPrincipal, 1);
        posicao(53, 2); cout << "Como Jogar";
        mudarCor(7); // branco

        for(int i=0; i<4; i++){ // imprime barra lateral
            posicao(89, i); cout << char(186);
        }

        corInf(hConsole, cursorInfMenuPrincipal, 2);
        posicao(97, 2); cout << "Sistema de Pontos";
        mudarCor(7); // branco

        posicao(0, 29); cout << "Pressione Z para voltar";

        // imprime informações quando o cursor estiver em cima de inimigos
       if(cursorInfMenuPrincipal == 0){
            posicao(0, 9); textoCentralizado("Universidade do Vale do Itajai");
            posicao(0, 13); textoCentralizado("Professor: Alex Luciano Roesler Rese");
            posicao(0, 17); textoCentralizado("Alunos: Henrique Gustavo Gonzaga Belli");
        }

        // imprime informações quando o cursor estiver em cima de mapa e itens
        if(cursorInfMenuPrincipal == 1){
            posicao(0, 7);  textoCentralizado("Seu objetivo e destruir todos os inimigos (W)");
            posicao(0, 9);  textoCentralizado("Apertando espaco, voce dispara um projetil");
            posicao(0, 10); textoCentralizado("O jogador so pode disparar um projetil por vez", 8);
            posicao(0, 12); textoCentralizado("Inimigos tambem disparam projeteis");
            posicao(0, 13); textoCentralizado("(cada um causa 1 de dano)", 8);
            posicao(0, 16); textoCentralizado("Perde/Ganha o jogo, se caso o jogador:");
            posicao(0, 17); textoCentralizado("Perder todas as vidas", 8);
            posicao(0, 18); textoCentralizado("/", 8);
            posicao(0, 19); textoCentralizado("Destruir todos os inimigos", 8);
        }

        // imprime informações quando o cursor estiver em cima de score
        if(cursorInfMenuPrincipal == 2){
            posicao(0, 7); textoCentralizado("CADA INIMIGO ELIMINADO, CONCEDE ENTRE 10 E 30 PONTOS:");
            posicao(0, 9); textoCentralizado("Inimigos inicialmente Vermelhos = 30 PONTOS", 4);
            posicao(0, 11); textoCentralizado("Inimigos inicialmente Amarelos = 20 PONTOS", 6);
            posicao(0, 13); textoCentralizado("Inimigos inicialmente Verdes = 10 PONTOS", 2);
            posicao(0, 15); textoCentralizado("Nada alem disso diminui ou aumenta os pontos", 8);
        }



        if(_kbhit()){ //Se estiver pressionando uma tecla
        teclaInfMenuPrincipal = getch(); //Recebe o valor da tecla pressionada

        cursorInfMenuPrincipalNew = cursorInfMenuPrincipal;
            switch (teclaInfMenuPrincipal) {
                case 75: case 'a': case 'A': cursorInfMenuPrincipalNew--;break; // Esquerda
                case 77: case 'd': case 'D': cursorInfMenuPrincipalNew++; break; // Direita
                case 'z': case 'Z': // fecha as informações
                    system("cls");
                    finalizarInfMenuPrincipal=true;
                break;
            }

            if(cursorInfMenuPrincipalNew>=0 && cursorInfMenuPrincipalNew<=2){
            cursorInfMenuPrincipal = cursorInfMenuPrincipalNew;
            limparTela(0, 0, 200, 29);
            }

            if(cursorInfMenuPrincipal != cursorAnterior){
                limparTela(37, 7, 55, 21);
                cursorAnterior = cursorInfMenuPrincipal;
            }
        }
    }
}

int escolherNave(){

    while(true){
        system("cls");

        posicao(0, 11); textoCentralizado("Escolha sua nave:", 9);
        posicao(0, 13); textoCentralizado("1 - Tiro Unico / Velocidade Maxima");
        posicao(0, 15); textoCentralizado("2 - Tiro Duplo / Velocidade Media");
        posicao(0, 17); textoCentralizado("3 - Tiro Triplo / Velocidade Minima");

        char tecla = getch();

        switch(tecla){
            case '1':
                while(true){
                    system("cls");
                    posicao(0, 11); textoCentralizado("Tem certeza que deseja escolher a nave 1?", 9);
                    posicao(0, 13); textoCentralizado("1 - Sim          2 - Nao");

                    char confirmar = getch();
                    if (confirmar == '1') {
                        system("cls");
                        return 1;
                    } else if (confirmar == '2') {
                        system("cls");
                        break;
                    }
                }
                break;

            case '2':
                while(true){
                    system("cls");
                    posicao(0, 11); textoCentralizado("Tem certeza que deseja escolher a nave 2?", 9);
                    posicao(0, 13); textoCentralizado("1 - Sim          2 - Nao");

                    char confirmar = getch();
                    if (confirmar == '1') {
                        system("cls");
                        return 2;
                    } else if (confirmar == '2') {
                        system("cls");
                        break;
                    }
                }
                break;

            case '3':
                while(true){
                    system("cls");
                    posicao(0, 11); textoCentralizado("Tem certeza que deseja escolher a nave 3?", 9);
                    posicao(0, 13); textoCentralizado("1 - Sim          2 - Nao");

                    char confirmar = getch();
                    if (confirmar == '1') {
                        system("cls");
                        return 3;
                    } else if (confirmar == '2') {
                        system("cls");
                        break;
                    }
                }
                break;
        }
    }
}

int escolherDificuldade(bool &finalizarMenuPrincipal){

    while(true){
        system("cls");

        posicao(0, 11); textoCentralizado("Escolha o nivel de dificuldade que deseja jogar:", 12);
        posicao(0, 13); textoCentralizado("1 - FACIL - Todos os inimigos possuem durabilidade minima");
        posicao(0, 15); textoCentralizado("2 - NORMAL - Durabilidade inimiga mista");
        posicao(0, 17); textoCentralizado("3 - DIFICIL - Todos os inimigos possuem durabilidade maxima");
        posicao(0, 28); cout << "Pressione qualquer outra tecla para retornar";

        char tecla = getch();

        switch(tecla){
            case '1':
                while(true){
                    system("cls");
                    posicao(0, 11); textoCentralizado("Tem certeza que deseja escolher a dificuldade FACIL?", 12);
                    posicao(0, 13); textoCentralizado("1 - Sim          2 - Nao");

                    char confirmar = getch();
                    if (confirmar == '1') {
                        finalizarMenuPrincipal = true;
                        return 1;
                    } else if (confirmar == '2') {
                        break;
                    }
                }
                break;

            case '2':
                while(true){
                    system("cls");
                    posicao(0, 11); textoCentralizado("Tem certeza que deseja escolher a dificuldade NORMAL?", 12);
                    posicao(0, 13); textoCentralizado("1 - Sim          2 - Nao");

                    char confirmar = getch();
                    if (confirmar == '1') {
                        finalizarMenuPrincipal = true;
                        return 2;
                    } else if (confirmar == '2') {
                        break; // <-- Sai da confirmação e volta para o menu principal
                    }
                }
                break;

            case '3':
                while(true){
                    system("cls");
                    posicao(0, 11); textoCentralizado("Tem certeza que deseja escolher a dificuldade DIFICIL?", 12);
                    posicao(0, 13); textoCentralizado("1 - Sim          2 - Nao");

                    char confirmar = getch();
                    if (confirmar == '1') {
                        finalizarMenuPrincipal = true;
                        return 3;
                    } else if (confirmar == '2') {
                        break;
                    }
                }
                break;

            default:
                return 0;
        }
    }
}

int printarMenuInicial(){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int cursorMenuPrincipal=0, cursorMenuPrincipalNew;
    char teclaMenuPrincipal;
    bool finalizarMenuPrincipal = false;
    int dificuldade = 0;

    while(!finalizarMenuPrincipal){
        mudarCor(7); // branco

        posicao(58, 11); cout << "Jogar";
        posicao(58-3, 13); cout << "Informacoes";
        posicao(58-4, 15); cout << "Ranking top 15";
        posicao(58, 17); cout << "Sair";

        mudarCor(4); //vermelho

        //imprime a seta onde estiver o cursor
        if(cursorMenuPrincipal==0){
            posicao(54, 11); cout << "==>";
        }else{
            posicao(54, 11); cout << "   ";
        }

        if(cursorMenuPrincipal==1){
            posicao(54-3, 13); cout << "==>";
        }else{
            posicao(54-3, 13);  cout << "   ";
        }

        if(cursorMenuPrincipal==2){
            posicao(54-4, 15); cout << "==>";
        }else{
            posicao(54-4, 15); cout << "   ";
        }
        if(cursorMenuPrincipal==3){
            posicao(54, 17); cout << "==>";
        }else{
            posicao(54, 17); cout << "   ";
        }


        if (_kbhit()) { //Se estiver pressionando uma tecla
            teclaMenuPrincipal = getch(); //Recebe o valor da tecla pressionada

            cursorMenuPrincipalNew = cursorMenuPrincipal;

            switch (teclaMenuPrincipal) {
                case 72: case 'w': case 'W': cursorMenuPrincipalNew--; break; // Cima
                case 80: case 's': case 'S': cursorMenuPrincipalNew++; break; // Baixo
                case 13: case ' ': //espaço ou enter para confirmar
                if(cursorMenuPrincipal==0){ // limpa a tela e finaliza a função
                    mudarCor(7); // branco
                    dificuldade = escolherDificuldade(finalizarMenuPrincipal);
                    system("cls");
                }else if(cursorMenuPrincipal==1){ // limpa a tela e abre informações
                    mudarCor(7); // branco
                    system("cls");
                    informacoesMenu();
                }else if(cursorMenuPrincipal==2){// limpa a tela e abre as 10 melhores pontuacoes
                    exibirTop15();
                    system("cls");
                }else if(cursorMenuPrincipal==3){ // finaliza o programa
                    mudarCor(7); // branco
                    exit(0);
                }
                break;
            }

            if(cursorMenuPrincipalNew>=0 && cursorMenuPrincipalNew<=3){
                cursorMenuPrincipal = cursorMenuPrincipalNew;
            }
        }
    }
    if (finalizarMenuPrincipal == true && dificuldade > 0){
        return dificuldade;
    }
}

void cronometro(auto &inicio, int &minuto = 0) {
    posicao(0, 27);
    // Espera por um segundo antes de mostrar o tempo
    auto agora = chrono::steady_clock::now();
    auto duracao = chrono::duration_cast<chrono::seconds>(agora - inicio);

    if(duracao.count() == 60) {
        inicio = chrono::steady_clock::now();
        minuto++;
    }
    cout << "Tempo: " << minuto << ":" ;

    if(duracao.count() < 10) {
        cout << "0";
    }
    cout << duracao.count() << endl;
}

void salvarPontuacao(Personagem personagem) {
    ofstream arquivo("pontuacao.txt", ios::app);
    if (arquivo.is_open()) {
        arquivo << personagem.nome << ": " << personagem.score << endl;
        arquivo.close();
    }
}

void fimDeJogo(Personagem& personagem, bool venceu) {

    system("cls");
    cout << "Fim de jogo! " << (venceu ? "Você venceu!" : "Você morreu!") << endl;
    cout << "Sua pontuação final foi de: " << personagem.score << " pts." << endl;

    cout << "Digite seu nome: ";
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    getline(cin, personagem.nome);

    if (personagem.nome == "apagarArquivo") {
        ofstream arquivo("pontuacao.txt", ios::trunc);
        arquivo.close();
        cout << "Pontuações resetadas!" << endl;
    } else if (personagem.nome == "rodadaTeste" || personagem.nome == ""){
        cout << "Sua pontuação nao foi salva!" << endl;
    }else {
        salvarPontuacao(personagem);
        cout << "Pontuação salva!" << endl;
    }
}

void printarJogo(Personagem personagem, int m[ALTURA][LARGURA]){
    posicao(0, 0);
    for (int i = 0; i < ALTURA; i++) {
        for (int j = 0; j < LARGURA; j++) {
            bool desenhou = false;

            for (auto& d : disparos) {
                if (d.ativo && d.coordenadasD.y == i && d.coordenadasD.x == j) {
                    if (d.direcao == 1){
                        cout << '!';
                        desenhou = true;
                        break;
                    } else {
                        mudarCor(9); //azul claro
                        cout << '|';
                        mudarCor(7);
                        desenhou = true;
                        break;
                    }
                }
            }

            if (!desenhou) {
                for (auto& inimigo : inimigos) {
                    if (inimigo.vivo && inimigo.coordenadasI.y == i && inimigo.coordenadasI.x == j) {
                        if (inimigo.vida == 3){
                            mudarCor(4);
                        } else if (inimigo.vida == 2){
                            mudarCor(6);
                        } else {
                            mudarCor(2);
                        }
                        cout << 'W'; // símbolo do inimigo
                        desenhou = true;
                        mudarCor(7);
                        break;
                    }
                }
            }

            if (!desenhou) {
                if (i == personagem.coordenadasP.y && j == personagem.coordenadasP.x){
                    mudarCor(9);
                    cout << '^';
                    mudarCor(7);
                    desenhou = true;
                } else {
                    switch (m[i][j]) {
                        case 0: cout << ' '; break;
                        case 1: cout << char(219); break;
                    }
                    desenhou = true;
                }
            }
        }
        cout << '\n';
    }

    for (auto it = efeitos.begin(); it != efeitos.end(); ) {
        posicao(it->x, it->y); // ou posicao(it->x, it->y), depende do seu código
        mudarCor(4); // vermelho, por exemplo
        cout << 'X';
        mudarCor(7); // voltar cor normal

        it->duracao--;
        if (it->duracao <= 0) {
            it = efeitos.erase(it); // remove efeito já terminado
        } else {
            ++it;
        }
    }
}

void atualizarDisparos() {

    if (tickGlobal % TICK_DISPAROS != 0) return;

    for (auto& d : disparos) {
        if (d.ativo) {
            d.coordenadasD.y += d.direcao;
            if (d.coordenadasD.y < 1 || d.coordenadasD.y >= ALTURA-1)
                d.ativo = false;
        }
    }

    disparos.erase(
        remove_if(disparos.begin(), disparos.end(),
            [](const Disparo& d) { return !d.ativo; }),
        disparos.end()
    );

}

void moverInimigos() {
    bool precisaDescer = false;

    // Move os inimigos na direção atual
    for (auto& inimigo : inimigos) {
        if (!inimigo.vivo) continue;
        inimigo.coordenadasI.x += direcaoInimigos;

        // Verifica se chegou na borda da tela
        if (inimigo.coordenadasI.x <= 1 || inimigo.coordenadasI.x >= LARGURA - 2) {
            precisaDescer = true;
        }
    }

    // Se qualquer inimigo bateu na borda, todos descem e invertem a direção
    if (precisaDescer) {
        direcaoInimigos *= -1;
        for (auto& inimigo : inimigos) {
            inimigo.coordenadasI.y += 1;
        }
    }
}

void ajustarVelocidadeInimigos() {
    int vivos = 0;
    for (auto& i : inimigos)
        if (i.vivo) vivos++;

    if (vivos == 0) return; // todos mortos — já ganhou

    // quanto menos inimigos, menor o intervalo
    tickMovInimigos = max(6, 20 * vivos / (int)inimigos.size());
}

void verificarColisoesEntreDisparos(){

    for (size_t i = 0; i < disparos.size(); ++i) {
        if (!disparos[i].ativo || disparos[i].direcao != -1) continue; // só do jogador

        for (size_t j = 0; j < disparos.size(); ++j) {
            if (!disparos[j].ativo || disparos[j].direcao != 1) continue; // só inimigos

            if (disparos[i].coordenadasD.x == disparos[j].coordenadasD.x &&
                disparos[i].coordenadasD.y == disparos[j].coordenadasD.y) {

                // Colisão entre os dois
                disparos[i].ativo = false;
                disparos[j].ativo = false;
                efeitos.push_back({disparos[i].coordenadasD.x, disparos[i].coordenadasD.y, 3});
                break; // esse disparo do jogador já colidiu, pode parar
            }
        }
    }
}

bool verificarColisoes(Personagem &jogador) {
    for (auto& d : disparos) {
        if (!d.ativo) continue;

        if (d.direcao == -1) { // Disparo do jogador
            for (auto& ini : inimigos) {
                if (ini.vivo &&
                    d.coordenadasD.x == ini.coordenadasI.x &&
                    d.coordenadasD.y == ini.coordenadasI.y) {

                    tocarBeep(100);
                    ini.vida--; // perde 1 de vida
                    d.ativo = false;

                    if (ini.vida <= 0) {
                        ini.vivo = false;
                        jogador.score += 10 * ini.tipo;
                        efeitos.push_back({ini.coordenadasI.x, ini.coordenadasI.y, 5});
                        ajustarVelocidadeInimigos();
                    }

                    break; // encerra o loop após o tiro atingir alguém
                }
            }

        } else if (d.direcao == 1) { // Disparo de inimigo
            if (d.coordenadasD.x == jogador.coordenadasP.x &&
                d.coordenadasD.y == jogador.coordenadasP.y) {

                tocarBeep(500);
                jogador.vida--;
                d.ativo = false;
                if (jogador.vida <= 0){
                    return true; // Game Over
                }
            }
        }
    }

    verificarColisoesEntreDisparos();

    // Verifica se algum inimigo encostou no jogador
    for (auto& ini : inimigos) {
        if (ini.vivo && ini.coordenadasI.y >= jogador.coordenadasP.y) {
            return true; // Game over
        }
    }

    return false;
}

void inicializarInimigos(int dificuldade) {
    inimigos.clear();
    int linhas = 4;
    int colunas = 5;
    int espacamentoX = 4;
    int espacamentoY = 1;

    for (int i = 0; i < linhas; ++i) {
        for (int j = 0; j < colunas; j++) {
            Inimigos ini;
            ini.coordenadasI.x = 1 + j * espacamentoX;
            ini.coordenadasI.y = 1 + i * espacamentoY;
            ini.vivo = true;

            if (dificuldade == 1){
                ini.tipo = 1;
            } else if (dificuldade == 2){
                // Define a vida com base na linha (i) caso a dificuldade seja NORMAL (2)
                if (i == 0) {         ini.tipo = 3;
                }else if (i == 1) {   ini.tipo = 2;
                }else {               ini.tipo = 1;
                }
            } else if (dificuldade == 3){
                ini.tipo = 3;
            }

            if (ini.tipo == 3) {         ini.vida = 3;
            }else if (ini.tipo == 2) {   ini.vida = 2;
            }else {                    ini.vida = 1;
            }

            inimigos.push_back(ini);
        }
    }
}

void inimigosAtiram(){

    int vivos = 0;
    for(auto& i : inimigos)
        if(i.vivo) vivos++;

    if (vivos == 0) return;

    int chance = max(100, 600 * vivos / (int)inimigos.size()); // mínimo 15

    for(auto& i : inimigos){
        if(i.vivo && rand()%chance == 0){
            Disparo d;
            d.coordenadasD.x = i.coordenadasI.x;
            d.coordenadasD.y = i.coordenadasI.y + 1;
            d.ativo = true;
            d.direcao = 1; // disparo inimigo vai para baixo
            disparos.push_back(d);
        }
    }
}

bool verificarVitoria(){

    for(auto& i : inimigos){
        if(i.vivo){
            return false;
        }
    }
    return true;
}

int main()
{
    ///ALERTA: NAO MODIFICAR O TRECHO DE CODIGO, A SEGUIR.
        //INICIO: COMANDOS PARA QUE O CURSOR NAO FIQUE PISCANDO NA TELA
        HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO     cursorInfo;
        GetConsoleCursorInfo(out, &cursorInfo);
        cursorInfo.bVisible = false; // set the cursor visibility
        SetConsoleCursorInfo(out, &cursorInfo);
        //FIM: COMANDOS PARA QUE O CURSOR NAO FIQUE PISCANDO NA TELA
        //INICIO: COMANDOS PARA REPOSICIONAR O CURSOR NO INICIO DA TELA
        short int CX=0, CY=0;
        COORD coord;
        coord.X = CX;
        coord.Y = CY;
        //FIM: COMANDOS PARA REPOSICIONAR O CURSOR NO INICIO DA TELA
    ///ALERTA: NAO MODIFICAR O TRECHO DE CODIGO, ACIMA.

    int m[ALTURA][LARGURA] = {};
    Personagem personagem {{LARGURA/2, ALTURA-2}, 3, 0}; // Variavel do personagem

    for (int i = 0; i < ALTURA; i++) {
        for (int j = 0; j < LARGURA; j++) {
            if (i == 0 || i == ALTURA - 1 || j == 0 || j == LARGURA - 1) {
                m[i][j] = 1; // parede
            } else {
                m[i][j] = 0; // caminho
            }
        }
    }

    int dificuldade = printarMenuInicial();
    int nave = escolherNave();

    inicializarInimigos(dificuldade);

    auto inicio = chrono::steady_clock::now();
    int minuto = 0;
    int maxVidas = 3; // pra desenhar vidas

    while(true){

        posicao(0, 26); cout << "Score: " << personagem.score;
        posicao(15, 27);
        cout << "Vida: ";
        for (int i = 0; i < maxVidas; i++) {
            if (i < personagem.vida)
                mudarCor(4); // vermelho
            else
                mudarCor(8); // cinza escuro ou apagado
            cout << char(219) << " ";
        }
        mudarCor(7); // volta pro branco

        tickGlobal++;

        cronometro(inicio, minuto);

        atualizarDisparos(); //atualizacao de todos os disparos na tela

        if (tickGlobal - tickUltimoMovInimigos >= tickMovInimigos) { /// movimentacao dos inimigos
            moverInimigos();
            tickUltimoMovInimigos = tickGlobal;
        }
        inimigosAtiram();

        printarJogo(personagem, m); //desenhar mapa, inimigos, disparos, personagem na tela

        switch (nave){
        case 1:
            TICK_MOV_JOGADOR = 2;
            break;
        case 2:
            TICK_MOV_JOGADOR = 4;
            break;
        case 3:
            TICK_MOV_JOGADOR = 8;
            break;
        }

        ///movimentacao/comandos
        if (tickGlobal - tickUltimoMovJogador >= TICK_MOV_JOGADOR) {
            if (GetAsyncKeyState(VK_LEFT) & 0x8000 || GetAsyncKeyState('A') & 0x8000) {
                movimentacaoPersonagem('A', personagem, m);
                tickUltimoMovJogador = tickGlobal;
            }
            else if (GetAsyncKeyState(VK_RIGHT) & 0x8000 || GetAsyncKeyState('D') & 0x8000) {
                movimentacaoPersonagem('D', personagem, m);
                tickUltimoMovJogador = tickGlobal;
            }
        }

        bool teclaEspacoAgora = (GetAsyncKeyState(VK_SPACE) & 0x8000);
        if (teclaEspacoAgora && !teclaEspacoAnterior) {
            if (tickGlobal - tickUltimoDisparoJogador >= TICK_INTERVALO_DISPARO) {
                dispararProjetil(personagem, nave);
                tickUltimoDisparoJogador = tickGlobal;
            }
        }
        teclaEspacoAnterior = teclaEspacoAgora;



        bool perdeu = verificarColisoes(personagem); //verificacao para ver se os inimigos tocaram em baixo ou o personagem perdeu todas as vidas

        if(perdeu){ // fim de jogo
            fimDeJogo(personagem, false); // Perdeu
            return 0;
        } else if (verificarVitoria()){
            fimDeJogo(personagem, true); // Ganhou
            return 0;
        }
    }

    return 0;
} //fim main
