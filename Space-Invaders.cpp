#include <iostream>
#include <windows.h>
#include <conio.h>
#include <chrono>
#include <thread>
#include <ctime>
#include <fstream>
#include <string>

#define ALTURA 25
#define LARGURA 25

#define INIMIGOALTURA 3
#define INIMIGOLARGURA 10

using namespace std;

bool projetilAtivo = false;
int px = -1, py = -1;
DWORD ultimoTiro = 0;

bool projetilAtivoInimigo = false;
int tiroinimigoX = -2,tiroinimigoY = -2;
DWORD tirodoinimigo = GetTickCount();
DWORD tirodoinimigo1 = GetTickCount();

bool deveDescer = false;

struct Ponto{
    int x, y;
};

struct StatusPersonagem{
    Ponto pontoPersonagem;
    int vida, score;
    string nome;
};

struct StatusInimigo{
    Ponto pontoInimigo;
    bool vivo;
};

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

int larguraConsole() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
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

//verifica o mapa
void imprimirmapa(int m[ALTURA][LARGURA], int i, int j){
    switch (m[i][j]){
    case 0: cout<<" "; break; //caminho
    case 1: cout<<char(219); break; //parede
    }
}

// verifica os inimigos
bool dentroDoInimigo (int i, int j, StatusInimigo inimigo){
    return( i >= inimigo.pontoInimigo.y &&
    i < inimigo.pontoInimigo.y + INIMIGOALTURA &&
    j >= inimigo.pontoInimigo.x &&
    j < inimigo.pontoInimigo.x + INIMIGOLARGURA);


}

// movimento do disparo
void atualizarProjetil(int m[ALTURA][LARGURA]) {
    if (projetilAtivo && GetTickCount() - ultimoTiro > 100) {
        py--;
        if (py < 0 or m[py][px] == 1) {
            projetilAtivo = false;
        }
        ultimoTiro = GetTickCount();
    }
}

// disparo 1 por vez
void dispararProjetil(StatusPersonagem personagem) {
    if (projetilAtivo == false) {
        projetilAtivo = true;
        py = personagem.pontoPersonagem.y - 1;
        px = personagem.pontoPersonagem.x;
        ultimoTiro = GetTickCount();
    }
}

//atualizasao do tiro do inimigo
void atualizarProjetilInimigo(int m[ALTURA][LARGURA]) {
    if (projetilAtivoInimigo == true && GetTickCount() - tirodoinimigo > 100/*a velocidade da bala*/) {
        tiroinimigoY++;
        if (tiroinimigoY < 0 or m[tiroinimigoY][tiroinimigoX] == 1/*onde a bala vai se desativada*/) {
            projetilAtivoInimigo = false;
        }
        tirodoinimigo = GetTickCount();
    }
}

//disparo do inimigo
void dispararProjetilinimigo(StatusInimigo inimigo, int i, int j) {
    if (projetilAtivoInimigo == false) {
        projetilAtivoInimigo  = true;
        tiroinimigoY = inimigo.pontoInimigo.y + 1 + i;
        tiroinimigoX = inimigo.pontoInimigo.x + j;
        tirodoinimigo = GetTickCount();
    }
}

void tiroinimigopermitido(StatusInimigo inimigo, int i, int j){
    if(GetTickCount() - tirodoinimigo1 > 1000){
        dispararProjetilinimigo(inimigo, i, j);
        tirodoinimigo1 = GetTickCount();
    }
}

void movimentacaopersonagem(char tecla, StatusPersonagem &personagem, int mapa[ALTURA][LARGURA]){

    int newX = personagem.pontoPersonagem.x;
    int newY = personagem.pontoPersonagem.y;

    switch (tecla) {
        case 75: case 'a': case 'A': newX--; break; // Esquerda
        case 77: case 'd': case 'D': newX++; break; // Direita
        case 32: dispararProjetil(personagem); break;
        default: break;
    }

    if(newX >= 0 && newX < LARGURA ) {
        if(mapa[newY][newX] != 1 && mapa[newY][newX] !=3) {  // Permite movimento em qualquer tile que não seja parede e tenha chão
            personagem.pontoPersonagem.x = newX;
        }
    }
}

//imprime o mapa
void imprimirMapaEInimigo (int m[ALTURA][LARGURA], int i, int j, StatusInimigo inimigo, int matinimigo[INIMIGOALTURA][INIMIGOLARGURA]){
     if (dentroDoInimigo(i,j,inimigo) && matinimigo[i - inimigo.pontoInimigo.y][j - inimigo.pontoInimigo.x] == 1) {
        mudarCor(12);
        cout << "W"; // ou qualquer caractere que represente o inimigo
        mudarCor(7);
    }else if (projetilAtivo && i == py && j == px){
        mudarCor(9);
        cout << "i";
        mudarCor(7);
    }else if (projetilAtivoInimigo && i == tiroinimigoY && j == tiroinimigoX){
    mudarCor(4);
    cout << "!"; // projétil inimigo
    mudarCor(7);
    }else{imprimirmapa(m, i, j);

    }
}

bool inimigoAtingiuFim(StatusInimigo inimigo, int matinimigo[INIMIGOALTURA][INIMIGOLARGURA]) {
    for (int i = 0; i < INIMIGOALTURA; i++) {
        for (int j = 0; j < INIMIGOLARGURA; j++) {
            if (matinimigo[i][j] == 1) {
                int y = inimigo.pontoInimigo.y + i;

                if (y >= ALTURA - 2) {
                    return true;
                }
            }
        }
    }
    return false;
}

//verifica atualiza a morte dos inimigos
void colisaoInimigo(StatusInimigo &inimigo, int matinimigo[INIMIGOALTURA][INIMIGOLARGURA], int &velocidadeInimigoY, int &velocidadeInimigoX, StatusPersonagem &personagem) {
    if (projetilAtivo == false){return;}

    int altura = INIMIGOALTURA;
    int largura = INIMIGOLARGURA;

    for (int i = 0; i < altura; i++) {
        for (int j = 0; j < largura; j++) {
            if (py == inimigo.pontoInimigo.y + i && px == inimigo.pontoInimigo.x + j && matinimigo[i][j] == 1) {
                matinimigo[i][j] = 0;
                velocidadeInimigoY -= 50;
                velocidadeInimigoX -= 50;
                personagem.score += 10;
                projetilAtivo = false;
                return;
            }
        }
    }
}

void verificarColisaoComInimigo(StatusPersonagem &personagem, StatusInimigo inimigo, int matinimigo[INIMIGOALTURA][INIMIGOLARGURA]) {
    for (int i = 0; i < INIMIGOALTURA; i++) {
        for (int j = 0; j < INIMIGOLARGURA; j++) {
            if (matinimigo[i][j] == 1) {
                int inimigoY = inimigo.pontoInimigo.y + i;
                int inimigoX = inimigo.pontoInimigo.x + j;

                if (personagem.pontoPersonagem.y == inimigoY &&
                    personagem.pontoPersonagem.x == inimigoX) {
                    personagem.vida--;
                    return;
                }
            }
        }
    }
}

//faz a movimentação dos inimigos
void movimentacaoInimigo(int tempoDesdeUltimoMovimentoY,
                         int tempoDesdeUltimoMovimentoX,
                         int velocidadeInimigoY,
                         int velocidadeInimigoX,
                         int &direcaoInimigo,
                         chrono::steady_clock::time_point &ultimoMovimentoInimigoY,
                         chrono::steady_clock::time_point &ultimoMovimentoInimigoX,
                         chrono::steady_clock::time_point &agora,
                         StatusInimigo &inimigo,
                         bool &deveDescer){

    if (deveDescer == true && tempoDesdeUltimoMovimentoY >= velocidadeInimigoY) {
        inimigo.pontoInimigo.y++;
        deveDescer = false;
        ultimoMovimentoInimigoY = agora;
        return;
    }

    int novoX = inimigo.pontoInimigo.x + direcaoInimigo;

    if (tempoDesdeUltimoMovimentoX >= velocidadeInimigoX) {
        if (novoX <= 0 or novoX + INIMIGOLARGURA >= LARGURA - 1) {
            deveDescer = true;
            direcaoInimigo *= -1;
        } else {
            inimigo.pontoInimigo.x = novoX;
        }
        ultimoMovimentoInimigoX = agora;
    }
}

void disparoInimigo(StatusInimigo inimigo, int matinimigo [INIMIGOALTURA][INIMIGOLARGURA]){

    for (int i = 0; i < INIMIGOALTURA; i++) {
        for (int j = 0; j < INIMIGOLARGURA; j++) {
            if(matinimigo[i][j] == 1){
                int disparoAleatorio = rand() % 4 + 1;
                if(disparoAleatorio == 3){
                    tiroinimigopermitido(inimigo, i, j);
                }
            }
        }
    }

}

void verificacaoDeVidaPersonagem (StatusPersonagem &personagem){
    for(int i = 0; i < ALTURA; i ++){
        for(int j = 0; j<LARGURA; j ++){
            if(projetilAtivoInimigo == true && tiroinimigoY == personagem.pontoPersonagem.y && tiroinimigoX == personagem.pontoPersonagem.x){
                projetilAtivoInimigo = false;
                personagem.vida--;
            }
        }
    }
}

void ordenarRankings(StatusPersonagem scores[], int inicio, int fim) {
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
    StatusPersonagem scores[MAX];
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
    posicao(0, 17); textoCentralizado("Alunos: Marlon Ranguet Zucchetti, Gabriel Paranagua, Henrique Gustavo Gonzaga Belli");
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
                    system("cls");
                    finalizarMenuPrincipal = true;
                }else if(cursorMenuPrincipal==1){ // limpa a tela e abre informações
                    system("cls");
                    informacoesMenu();
                }else if(cursorMenuPrincipal==2){// limpa a tela e abre as 10 melhores pontuacoes
                    exibirTop10();
                    system("cls");
                }else if(cursorMenuPrincipal==3){ // finaliza o programa
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
    cout << duracao.count() << "                                                                 " << endl;
}

void salvarPontuacao(StatusPersonagem personagem) {
    ofstream arquivo("pontuacao.txt", ios::app);
    if (arquivo.is_open()) {
        arquivo << personagem.nome << ": " << personagem.score << endl;
        arquivo.close();
    }
}

void fimDeJogo(StatusPersonagem& personagem, bool venceu) {
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

    auto ultimoMovimentoInimigoY = chrono::steady_clock::now();
    auto ultimoMovimentoInimigoX = chrono::steady_clock::now();
    srand(time(NULL));

    int m[ALTURA][LARGURA] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    };

    StatusPersonagem personagem {{2, 20}, 3}; // Variavel do personagem
    StatusInimigo inimigo {2, 2};
    char tecla;

    int matinimigo [INIMIGOALTURA][INIMIGOLARGURA];

    for(int i = 0; i<INIMIGOALTURA; i++){
        for(int j = 0; j<INIMIGOLARGURA; j++){
            matinimigo[i][j] = 1;
        }
    }

    int velocidadeInimigoY = 3000;
    int velocidadeInimigoX = 1750;
    int direcao = -1;

    int ultimaVidaDesenhada = -1;
    int ultimoScoreDesenhado = -1;

    int minuto = 0;

    printarMenuInicial();

    auto inicio = chrono::steady_clock::now();

    while(true){

        cronometro(inicio, minuto);

        auto agora = chrono::steady_clock::now();
        auto tempopassado = chrono::duration_cast<chrono::milliseconds>(agora - inicio).count();
        auto tempoDesdeUltimoMovimentoY = chrono::duration_cast<chrono::milliseconds>(agora - ultimoMovimentoInimigoY).count();
        auto tempoDesdeUltimoMovimentoX = chrono::duration_cast<chrono::milliseconds>(agora - ultimoMovimentoInimigoX).count();

        movimentacaoInimigo(tempoDesdeUltimoMovimentoY,tempoDesdeUltimoMovimentoX,velocidadeInimigoY,velocidadeInimigoX,direcao,ultimoMovimentoInimigoY,ultimoMovimentoInimigoX, agora, inimigo, deveDescer);

        ///Posiciona a escrita no iicio do console
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

        atualizarProjetil(m);
        colisaoInimigo(inimigo, matinimigo, velocidadeInimigoY, velocidadeInimigoX, personagem);
        verificarColisaoComInimigo(personagem, inimigo, matinimigo);
        disparoInimigo(inimigo, matinimigo);
        atualizarProjetilInimigo(m);
        verificacaoDeVidaPersonagem(personagem);

        for(int i=0;i<ALTURA;i++){
            for(int j=0;j<LARGURA;j++){
                if(i==personagem.pontoPersonagem.y && j==personagem.pontoPersonagem.x){
                    mudarCor(2);
                    cout<< "A"; //personagem
                    mudarCor(7);
                }else {
                    imprimirMapaEInimigo(m,i,j,inimigo,matinimigo);
                }
            }
            cout<<"\n";
        } //fim for mapa

        ///executa os movimentos
         if ( _kbhit() ){
            tecla = getch();
            movimentacaopersonagem(tecla, personagem, m);
         }

        this_thread::sleep_until(inicio + chrono::milliseconds(20));

        if(personagem.vida != ultimaVidaDesenhada){
            posicao(0, 25);
            cout << "               ";
            posicao(0, 25);
            cout << "vida:";
            mudarCor(12);
            for(int i = 0; i < personagem.vida; i++){
            cout << char(219);
        }
        mudarCor(7);
        ultimaVidaDesenhada = personagem.vida;
    }

        if(personagem.score != ultimoScoreDesenhado){
            posicao(20, 25);
            cout << "                ";
            posicao(20, 25);
            cout << "score: ";
            mudarCor(14);
            cout << personagem.score;
            mudarCor(7);

            ultimoScoreDesenhado = personagem.score;
        }


        bool venceu = true;
        for (int i = 0; i < INIMIGOALTURA; i++) {
            for (int j = 0; j < INIMIGOLARGURA; j++) {
                if (matinimigo[i][j] == 1) {
                    venceu = false;
                    break;
                }
            }
            if (!venceu) break;
        }

        if(inimigoAtingiuFim(inimigo, matinimigo) || venceu){
            fimDeJogo(personagem, true);
            return 0;
        } else if(personagem.vida == 0){
            fimDeJogo(personagem, false);
            return 0;
        }

    } //fim do laco do jogo

    return 0;
} //fim main
