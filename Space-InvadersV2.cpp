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

#define ALTURA 25
#define LARGURA 35

using namespace std;

struct Ponto{
    int x, y;
};

struct Personagem{
    Ponto coordenadasP;
    int vida, score;
    string nome;
};

struct Inimigos{
    Ponto coordenadasI;
    bool vivo;
    int vida;
};

struct Disparo {
    Ponto coordenadasD;
    bool ativo;  // se disparo está ativo na tela
    int direcao; // 1 para disparo inimigo (pra baixo), -1 para disparo jogador (pra cima)
};

vector<Inimigos> inimigos;
vector<Disparo> disparos;


int tickGlobal = 0;
int tickMovInimigos            = 15;
const int TICK_MOV_JOGADOR     = 2;
const int TICK_DISPAROS        = 1;
const int TICK_INTERVALO_DISPARO = 10;

int tickUltimoMovJogador   = 0;
int tickUltimoMovInimigos  = 0;
int tickUltimoDisparo      = -TICK_INTERVALO_DISPARO;

int direcaoInimigos = 1;

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

//Função para limpar apenas uma area da tela
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

void dispararProjetil(const Personagem& personagem) {

    if (tickGlobal - tickUltimoDisparo < 10) return; // intervalo de 10 ticks
    tickUltimoDisparo = tickGlobal;

    Disparo d;
    d.coordenadasD.x = personagem.coordenadasP.x;
    d.coordenadasD.y = personagem.coordenadasP.y - 1;
    d.direcao = -1;
    d.ativo = true;
    disparos.push_back(d);
}

void movimentacaoPersonagem(char tecla, Personagem &personagem, int mapa[ALTURA][LARGURA]){

    int newX = personagem.coordenadasP.x;

    switch (tecla) {
        case 75: case 'a': case 'A': newX--; break; // Esquerda
        case 77: case 'd': case 'D': newX++; break; // Direita
        case 32: dispararProjetil(personagem); break;
        default: break;
    }

    if(newX >= 0 && newX < LARGURA) {
        if(mapa[personagem.coordenadasP.y][newX] != 1) {  // Permite movimento em qualquer tile que não seja parede e tenha chão
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

void exibirTop10() {
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

    textoCentralizado("=== TOP 10 ===", 5);

    cout << endl << endl;

    for (int i = 0; i < min(10, contador); i++) {
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
            posicao(0, 17); textoCentralizado("Alunos: Marlon Ranguet Zucchetti, Gabriel Paranagua, Henrique Gustavo Gonzaga Belli");
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
            posicao(0, 7); textoCentralizado("CADA INIMIGO ELIMINADO, CONCEDE 10 PONTOS");
            posicao(0, 8); textoCentralizado("Nada alem disso diminui ou aumenta os pontos", 8);
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

void creditos(){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    system("cls");

    posicao(0, 9); textoCentralizado("Universidade do Vale do Itajai");
    posicao(0, 13); textoCentralizado("Professor: Alex Luciano Roesler Rese");
    posicao(0, 17); textoCentralizado("Aluno: Henrique Gustavo Gonzaga Belli");
    posicao(0, 29);

    system("pause");

}

int printarMenuInicial(){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int cursorMenuPrincipal=0, cursorMenuPrincipalNew;
    char teclaMenuPrincipal;
    bool finalizarMenuPrincipal = false;

    while(!finalizarMenuPrincipal){
        mudarCor(7); // branco

        posicao(58, 11); cout << "Jogar";
        posicao(58-3, 13); cout << "Informacoes";
        posicao(58-4, 15); cout << "Ranking top 10";
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
                    system("cls");
                    finalizarMenuPrincipal = true;
                }else if(cursorMenuPrincipal==1){ // limpa a tela e abre informações
                    mudarCor(7); // branco
                    system("cls");
                    informacoesMenu();
                }else if(cursorMenuPrincipal==2){// limpa a tela e abre as 10 melhores pontuacoes
                    exibirTop10();
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
    cin >> personagem.nome;

    if (personagem.nome == "apagarArquivo") {
        ofstream arquivo("pontuacao.txt", ios::trunc);
        arquivo.close();
        cout << "Pontuações resetadas!" << endl;
    } else if (personagem.nome == "rodadaTeste"){
        cout << "Sua pontuação nao foi salva!" << endl;
    }else {
        salvarPontuacao(personagem);
        cout << "Pontuação salva!" << endl;
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
    tickMovInimigos = max(2, 10 * vivos / (int)inimigos.size());
}

bool verificarColisoes(Personagem &jogador) {
    for (auto& d : disparos) {
        if (!d.ativo) continue;

        if (d.direcao == -1) { // Disparo do jogador
            for (auto& ini : inimigos) {
                if (ini.vivo &&
                    d.coordenadasD.x == ini.coordenadasI.x &&
                    d.coordenadasD.y == ini.coordenadasI.y) {

                    ini.vida--; // perde 1 de vida
                    d.ativo = false;

                    if (ini.vida <= 0) {
                        ini.vivo = false;
                        jogador.score += 10;
                        ajustarVelocidadeInimigos();
                    }

                    break; // encerra o loop após o tiro atingir alguém
                }
            }

        } else if (d.direcao == 1) { // Disparo de inimigo
            if (d.coordenadasD.x == jogador.coordenadasP.x &&
                d.coordenadasD.y == jogador.coordenadasP.y) {

                jogador.vida--;
                d.ativo = false;
                if (jogador.vida <= 0){
                    return true; // Game Over
                }
            }
        }
    }

    // Verifica se algum inimigo encostou no jogador
    for (auto& ini : inimigos) {
        if (ini.vivo && ini.coordenadasI.y >= jogador.coordenadasP.y) {
            return true; // Game over
        }
    }

    return false;
}

void inicializarInimigos() {
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

            // Define a vida com base na linha (i)
            if (i == 0)
                ini.vida = 3;
            else if (i == 1)
                ini.vida = 2;
            else
                ini.vida = 1;

            inimigos.push_back(ini);
        }
    }
}

void inimigosAtiram(){

    int vivos = 0;
    for(auto& i : inimigos)
        if(i.vivo) vivos++;

    if (vivos == 0) return;

    int chance = max(30, 150 * vivos / (int)inimigos.size()); // mínimo 15

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
}

bool verificarDerrota(Personagem jogador){
    for(auto& i : inimigos){
        if(i.vivo && i.coordenadasI.y >= jogador.coordenadasP.y){
            return true;
        }
    }
    return false;
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

    printarMenuInicial();

    inicializarInimigos();

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

        atualizarDisparos();

        if (tickGlobal - tickUltimoMovInimigos >= tickMovInimigos) {
            moverInimigos();
            inimigosAtiram();
            tickUltimoMovInimigos = tickGlobal;
        }

        printarJogo(personagem, m);

        if (_kbhit()) {
            if (tickGlobal - tickUltimoMovJogador >= TICK_MOV_JOGADOR) {
                char tecla = getch();
                movimentacaoPersonagem(tecla, personagem, m);
                tickUltimoMovJogador = tickGlobal;
            }
        }

        bool perdeu = verificarColisoes(personagem);

        if(perdeu){ // fim de jogo
            fimDeJogo(personagem, false);
            return 0;
        } else if (verificarVitoria()){
            fimDeJogo(personagem, true);
            return 0;
        }
    }

    return 0;
} //fim main
