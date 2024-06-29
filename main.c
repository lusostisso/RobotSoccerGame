#include <math.h>
#include <chipmunk.h>
#include <SOIL.h>

// Rotinas para acesso da OpenGL
#include "opengl.h"

// Funções para movimentação de objetos
void moveRobo(cpBody* body, void* data);
void moveBola(cpBody* body, void* data);
void moveGoleiroEsquerda(cpBody* body, void* data);
void moveGoleiroDireita(cpBody* body, void* data);
void moveAtacanteEsquerda(cpBody* body, void* data);

// Prototipos
void initCM();
void freeCM();
void restartCM();
cpShape* newLine(cpVect inicio, cpVect fim, cpFloat fric, cpFloat elast);
cpBody* newCircle(cpVect pos, cpFloat radius, cpFloat mass, char* img, bodyMotionFunc func, cpFloat fric, cpFloat elast);

// Score do jogo
int score1 = 0;
int score2 = 0;

// Flag de controle: 1 se o jogo tiver acabado
int gameOver = 0;

// cpVect e' um vetor 2D e cpv() e' uma forma rapida de inicializar ele.
cpVect gravity;

// O ambiente
cpSpace* space;

// Paredes "invisíveis" do ambiente
cpShape* leftWall, *rightWall, *topWall, *bottomWall, *goleiraDireitaCima, *goleiraDireiraBaixo, *goleiraEsquerdaCima, *goleiraEsquerdaBaixo;

// A bola
cpBody* ballBody;

// Um robô
cpBody* robotBody;

// Jogadores time amora
cpBody* goleiroAmora, *defensorAmora, *atacanteAmora;

// Jogadores time framboesa
cpBody* goleiroFramboesa, *defensorFramboesa, *atacanteFramboesa;

//Variaveis global de coordenadas time amora
cpVect goleiroAmora_coordenada, defensorAmora_coordenada, atacanteAmora_coordenada;

//Variaveis global de coordenadas time framboesa
cpVect goleiroFramboesa_coordenada, defensorFramboesa_coordenada, atacanteFramboesa_coordenada;

// Cada passo de simulação é 1/60 seg.
cpFloat timeStep = 1.0/60.0;

// Inicializa o ambiente: é chamada por init() em opengl.c, pois necessita do contexto
// OpenGL para a leitura das imagens
void initCM()
{
    gravity = cpv(0, 100);

    // Cria o universo
    space = cpSpaceNew();

    // Seta o fator de damping, isto é, de atrito do ar
    cpSpaceSetDamping(space, 0.8); //colocaram 1 no outro

    // Descomente a linha abaixo se quiser ver o efeito da gravidade!
    //cpSpaceSetGravity(space, gravity);

    // Adiciona 4 linhas estáticas para formarem as "paredes" do ambiente
    leftWall   = newLine(cpv(0,0), cpv(0,ALTURA_JAN), 0, 1.0);
    rightWall  = newLine(cpv(LARGURA_JAN,0), cpv(LARGURA_JAN,ALTURA_JAN), 0, 1.0);
    bottomWall = newLine(cpv(0,0), cpv(LARGURA_JAN,0), 0, 1.0);
    topWall    = newLine(cpv(0,ALTURA_JAN), cpv(LARGURA_JAN,ALTURA_JAN), 0, 1.0);
    goleiraDireitaCima = newLine(cpv(976,325), cpv(LARGURA_JAN, 326), 0, 1.0);
    goleiraDireiraBaixo = newLine(cpv(976,386), cpv(LARGURA_JAN, 386), 0, 1.0);
    goleiraEsquerdaCima = newLine(cpv(0,325), cpv(47, 326), 0, 1.0);
    goleiraEsquerdaBaixo = newLine(cpv(0,386), cpv(77, 386), 0, 1.0);

    // Agora criamos a bola...
    // Os parâmetros são:
    //   - posição: cpVect (vetor: x e y)
    //   - raio
    //   - massa
    //   - imagem a ser carregada
    //   - ponteiro para a função de movimentação (chamada a cada passo, pode ser NULL)
    //   - coeficiente de fricção
    //   - coeficiente de elasticidade
    ballBody = newCircle(cpv(512,350), 8, 1, "small_football.png", moveBola, 0.2, 1);

    goleiroFramboesa_coordenada = cpv(100,356);
    goleiroAmora_coordenada = cpv(924,356);


    // ... e um robô de exemplo
    //robotBody = newCircle(cpv(50,350), 20, 5, "ship1.png", moveRobo, 0.2, 1);

    goleiroAmora = newCircle(goleiroFramboesa_coordenada, 15, 5, "monkey.png", moveGoleiroEsquerda, 0.2, 0);
    goleiroFramboesa = newCircle(goleiroAmora_coordenada, 15, 5, "monkey.png", moveGoleiroDireita, 0.2, 0);

    atacanteAmora = newCircle(cpv(200,717), 15, 5, "monkey.png", moveAtacanteEsquerda, 0.2, 0);
    
    /*
    defensorAmora = newCircle(cpv(256,180), 20, 5, "ship3.png", moveZagueiroEsquerda1, 0.2, 0);
    atacanteAmora = newCircle(cpv(440,317), 20, 5, "ship3.png", moveZagueiroEsquerda2, 0.2, 0);
    
    goleiroFramboesa = newCircle(cpv(924,356), 20, 5, "ship1.png", moveGoleiroDireita, 0.2, 0);
    defensorFramboesa = newCircle(cpv(770,180), 20, 5, "ship3.png", moveZagueiroDireita1, 0.2, 0);
    atacanteFramboesa = newCircle(cpv(585,317), 20, 5, "ship3.png", moveZagueiroDireita2, 0.2, 0);
    */

    //robotBody = newCircle(cpv(50,350), 20, 5, "monkey.png", moveAtacante, 0.2, 0.5);
}

void moveGoleiroEsquerda(cpBody* body, void* data){
    srand(time(0));

    cpVect velocidade = cpvclamp(cpBodyGetVelocity(body), (5+(rand() % 11))); //Define a velocidade em um intervalo aleatório entre 5 e 15.
    cpBodySetVelocity(body, velocidade);

    cpVect robotPos = cpBodyGetPosition(body); 
    cpVect ballPos  = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos,pos);
    delta = cpvmult(cpvnormalize(delta),20);

    if(ballPos.x < 189 && ballPos.y < 533 && ballPos.y > 180){
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    } else{
        cpVect deltaIni = cpvadd(goleiroFramboesa_coordenada,pos);
        cpBodyApplyImpulseAtWorldPoint(body, deltaIni, goleiroFramboesa_coordenada);
        if (fabs(robotPos.x - goleiroFramboesa_coordenada.x) < 0.1 && fabs(robotPos.y - goleiroFramboesa_coordenada.y) < 0.1){
             cpBodySetVelocity(body, cpv(0,0));
        }
    }
}

void moveGoleiroDireita(cpBody* body, void* data){
    srand(time(0));
    cpVect vel = cpBodyGetVelocity(body);
    vel = cpvclamp(vel, (5+(rand() % 11)));
    cpBodySetVelocity(body, vel);

    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos,pos);
    delta = cpvmult(cpvnormalize(delta), 20);

    if(ballPos.x > 835 && robotPos.y < 533 && robotPos.y > 180){
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    } else{
        cpVect deltaIni = cpvadd(goleiroAmora_coordenada, pos);
        cpBodyApplyImpulseAtWorldPoint(body, deltaIni, goleiroAmora_coordenada);
        if (fabs(robotPos.x - goleiroAmora_coordenada.x) < 0.1 && fabs(robotPos.y - goleiroAmora_coordenada.y) < 0.1 ){
             cpBodySetVelocity(body, cpv(0,0));
        }
    }
}

void moveAtacanteEsquerda(cpBody* body, void* data) {
    cpVect vel = cpBodyGetVelocity(body);
    srand(time(0));
    vel = cpvclamp(vel, (5 + (rand() % 11)));
    cpBodySetVelocity(body, vel);

    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos, pos);
    delta = cpvmult(cpvnormalize(delta), 20);

    cpFloat distance = cpvdist(robotPos, ballPos);

    if (distance < 28) {
        cpVect chute = cpvmult(cpvnormalize(cpvsub(goleiroAmora_coordenada, ballPos)), 10);
        cpBodyApplyImpulseAtWorldPoint(ballBody, chute, ballPos);
    }

    if (ballPos.x > 430 && ballPos.y <= 356) {
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    } else {
        cpBodySetVelocity(body, cpv(0,0));
    }
}

// Exemplo de função de movimentação: move o robô em direção à bola
void moveRobo(cpBody* body, void* data)
{
    // Veja como obter e limitar a velocidade do robô...
    cpVect vel = cpBodyGetVelocity(body);
//    printf("vel: %f %f", vel.x,vel.y);

    // Limita o vetor em 50 unidades
    vel = cpvclamp(vel, 50);
    // E seta novamente a velocidade do corpo
    cpBodySetVelocity(body, vel);

    // Obtém a posição do robô e da bola...
    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);

    // Calcula um vetor do robô à bola (DELTA = B - R)
    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos,pos);

    // Limita o impulso em 20 unidades
    delta = cpvmult(cpvnormalize(delta),20);
    // Finalmente, aplica impulso no robô
    cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
}

void moveBola(cpBody* body, void* data)
{
    cpVect ballPos  = cpBodyGetPosition(body);
    if(ballPos.x < 45 && ballPos.y > 326 && ballPos.y < 386){
        score2++;
        cpBodySetPosition(body, cpv(512,350));
        cpBodySetVelocity(body, cpv(0,0));
    }

    if(ballPos.x > 978 && ballPos.y > 326 && ballPos.y < 386){
        score1++;
        cpBodySetPosition(body, cpv(512,350));
        cpBodySetVelocity(body, cpv(0,0));
    }
}

// Libera memória ocupada por cada corpo, forma e ambiente
// Acrescente mais linhas caso necessário
void freeCM()
{
    printf("Cleaning up!\n");
    UserData* ud = cpBodyGetUserData(ballBody);
    cpShapeFree(ud->shape);
    cpBodyFree(ballBody);

    ud = cpBodyGetUserData(robotBody);
    cpShapeFree(ud->shape);
    cpBodyFree(robotBody);

    cpShapeFree(leftWall);
    cpShapeFree(rightWall);
    cpShapeFree(bottomWall);
    cpShapeFree(topWall);

    cpSpaceFree(space);
}

// Função chamada para reiniciar a simulação
void restartCM()
{
    // Escreva o código para reposicionar os jogadores, ressetar o score, etc.

    // Não esqueça de ressetar a variável gameOver!
    gameOver = 0;
}

// ************************************************************
//
// A PARTIR DESTE PONTO, O PROGRAMA NÃO DEVE SER ALTERADO
//
// A NÃO SER QUE VOCÊ SAIBA ***EXATAMENTE*** O QUE ESTÁ FAZENDO
//
// ************************************************************

int main(int argc, char** argv)
{
    // Inicialização da janela gráfica
    init(argc,argv);

    // Não retorna... a partir daqui, interação via teclado e mouse apenas, na janela gráfica
    glutMainLoop();
    return 0;
}

// Cria e adiciona uma nova linha estática (segmento) ao ambiente
cpShape* newLine(cpVect inicio, cpVect fim, cpFloat fric, cpFloat elast)
{
   cpShape* aux = cpSegmentShapeNew(cpSpaceGetStaticBody(space), inicio, fim, 0);
   cpShapeSetFriction(aux, fric);
   cpShapeSetElasticity(aux, elast);
   cpSpaceAddShape(space, aux);
   return aux;
}

// Cria e adiciona um novo corpo dinâmico, com formato circular
cpBody* newCircle(cpVect pos, cpFloat radius, cpFloat mass, char* img, bodyMotionFunc func, cpFloat fric, cpFloat elast)
{
    // Primeiro criamos um cpBody para armazenar as propriedades fisicas do objeto
    // Estas incluem: massa, posicao, velocidade, angulo, etc do objeto
    // A seguir, adicionamos formas de colisao ao cpBody para informar o seu formato e tamanho

    // O momento de inercia e' como a massa, mas para rotacao
    // Use as funcoes cpMomentFor*() para calcular a aproximacao dele
    cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);

    // As funcoes cpSpaceAdd*() retornam o que voce esta' adicionando
    // E' conveniente criar e adicionar um objeto na mesma linha
    cpBody* newBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));

    // Por fim, ajustamos a posicao inicial do objeto
    cpBodySetPosition(newBody, pos);

    // Agora criamos a forma de colisao do objeto
    // Voce pode criar multiplas formas de colisao, que apontam ao mesmo objeto (mas nao e' necessario para o trabalho)
    // Todas serao conectadas a ele, e se moverao juntamente com ele
    cpShape* newShape = cpSpaceAddShape(space, cpCircleShapeNew(newBody, radius, cpvzero));
    cpShapeSetFriction(newShape, fric);
    cpShapeSetElasticity(newShape, elast);

    UserData* newUserData = malloc(sizeof(UserData));
    newUserData->tex = loadImage(img);
    newUserData->radius = radius;
    newUserData->shape= newShape;
    newUserData->func = func;
    cpBodySetUserData(newBody, newUserData);
    printf("newCircle: loaded img %s\n", img);
    return newBody;
}
