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

using namespace std;

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

void movimentacaoPersonagem(char tecla, StatusPersonagem &personagem, int mapa[ALTURA][LARGURA]){

    int newX = personagem.pontoPersonagem.x;

    switch (tecla) {
        case 75: case 'a': case 'A': newX--; break; // Esquerda
        case 77: case 'd': case 'D': newX++; break; // Direita
        case 32: //dispararProjetil(personagem);
        break;
        default: break;
    }

    if(newX >= 0 && newX < LARGURA) {
        if(mapa[personagem.pontoPersonagem.y][newX] != 1) {  // Permite movimento em qualquer tile que não seja parede e tenha chão
            personagem.pontoPersonagem.x = newX;
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

    int m[ALTURA][LARGURA] = {};
    StatusPersonagem personagem {{2, 22}, 3}; // Variavel do personagem

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

    auto inicio = chrono::steady_clock::now();
    int minuto = 0;

    while(true){
        cronometro(inicio, minuto);

        posicao(0, 0);
        for (int i = 0; i < ALTURA; i++) {
            for (int j = 0; j < LARGURA; j++) {
                if (i == personagem.pontoPersonagem.y && j == personagem.pontoPersonagem.x) {
                    cout << 'O'; // personagem
                } else {
                    switch (m[i][j]) {
                        case 0: cout << ' '; break;      // caminho
                        case 1: cout << char(219); break; // parede
                    }
                }
            }
            cout << '\n';
        }

        if ( _kbhit() ){
            char tecla = getch();
            movimentacaoPersonagem(tecla, personagem, m);
         }
    }


    return 0;
} //fim main
