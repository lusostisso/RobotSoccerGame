#include <math.h>
#include <chipmunk.h>
#include <SOIL.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <stdbool.h>

// Rotinas para acesso da OpenGL
#include "opengl.h"

void isTheGameOver ();
void *isTheBallStuck(void *arg);
void checkAndApplyBoundaryImpulse(cpBody* body, void* data);
void resetPositionPlayers ();

// Funções para movimentação de objetos
void moveBola(cpBody* body, void* data);
void moveGoleiro(cpBody* body, void* data);
void moveAtacante(cpBody* body, void* data);
void moveDefensor (cpBody* body, void* data);

cpVect obterPosicaoAtacante(lado lado, int atacante);
void aplicarImpulsoNaBola(cpVect mira, cpVect ballPos);

jogador_data init_jogador_data(cpVect pos_inicial, lado lado){
    jogador_data j = {pos_inicial, lado};
    return j;
}

// Prototipos
void initCM();
void freeCM();
void restartCM();
cpShape* newLine(cpVect inicio, cpVect fim, cpFloat fric, cpFloat elast);
cpBody* newCircle(cpVect pos, cpFloat radius, cpFloat mass, char* img, bodyMotionFunc func, cpFloat fric, cpFloat elast,  jogador_data j);

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
cpShape* leftWall, *rightWall, *topWall, *bottomWall, *goleiraDireitaCima, *goleiraDireitaBaixo, *goleiraEsquerdaCima, *goleiraEsquerdaBaixo;

// A bola
cpBody* ballBody;

cpBody* goleiroDireita, *zagueiroDireita1, *zagueiroDireita2, *zagueiroDireita3, *atacanteDireita1, *atacanteDireita2, *goleiroEsquerda, *zagueiroEsquerda1, *zagueiroEsquerda2, *zagueiroEsquerda3, *atacanteEsquerda1, *atacanteEsquerda2;

//Variaveis global de coordenadas time 1
cpVect golEsq, defeEsq1, defeEsq2, defeEsq3, ataEsq1, ataEsq2, goleiraEsq, golDir, defeDir1, defeDir2, defeDir3, ataDir1, ataDir2, goleiraDir;
cpVect centroDoCampo;

// Cada passo de simulação é 1/60 seg.
cpFloat timeStep = 1.0/60.0;
pthread_t thread_id;

void initCM()
{
    // Cria o universo
    space = cpSpaceNew();

    // Seta o fator de damping, isto é, de atrito do ar
    cpSpaceSetDamping(space, 0.8);

    // Adiciona 4 linhas estáticas para formarem as "paredes" do ambiente
    leftWall   = newLine(cpv(0,0), cpv(0,ALTURA_JAN), 0, 1.0);
    rightWall  = newLine(cpv(LARGURA_JAN,0), cpv(LARGURA_JAN,ALTURA_JAN), 0, 1.0);
    bottomWall = newLine(cpv(0,0), cpv(LARGURA_JAN,0), 0, 1.0);
    topWall    = newLine(cpv(0,ALTURA_JAN), cpv(LARGURA_JAN,ALTURA_JAN), 0, 1.0);

    // Posicionamento inicial dos jogadores
    // campo : 1024 x 712
    golEsq = cpv(101,350);
    defeEsq1 = cpv(256,180);
    defeEsq2 = cpv(324,356);
    defeEsq3 = cpv(256,534);
    ataEsq1 = cpv(440,317);
    ataEsq2 = cpv(440,395);
    goleiraEsq = cpv(35, 356);
    golDir = cpv(923,350);
    defeDir1 = cpv(770,180);
    defeDir2 = cpv(700,356);
    defeDir3 = cpv(770,534);
    ataDir1 = cpv(585,317);
    ataDir2 = cpv(586,396);
    goleiraDir = cpv(987, 356);
    centroDoCampo = cpv(512,350);

    goleiraDireitaCima = newLine(cpv(976,325), cpv(LARGURA_JAN, 326), 0, 1.0);
    goleiraDireitaBaixo = newLine(cpv(976,386), cpv(LARGURA_JAN, 386), 0, 1.0);
    goleiraEsquerdaCima = newLine(cpv(0,325), cpv(47, 326), 0, 1.0);
    goleiraEsquerdaBaixo = newLine(cpv(0,386), cpv(77, 386), 0, 1.0);
 
    //Bola
    ballBody = newCircle(centroDoCampo, 8, 1, "small_football.png", moveBola, 0.2, 1, init_jogador_data(centroDoCampo, LEFT));

    goleiroEsquerda = newCircle(golEsq, 11, 5, "rosa.png", moveGoleiro, 0.2, 2, init_jogador_data(golEsq, LEFT));
    zagueiroEsquerda1 = newCircle(defeEsq1, 15, 5, "rosa.png", moveDefensor, 0.2, 0, init_jogador_data(defeEsq1, LEFT));
    zagueiroEsquerda2 = newCircle(defeEsq2, 15, 5, "rosa.png", moveDefensor, 0.2, 0, init_jogador_data(defeEsq2, LEFT));
    zagueiroEsquerda3 = newCircle(defeEsq3, 15, 5, "rosa.png", moveDefensor, 0.2, 0, init_jogador_data(defeEsq3, LEFT));
    atacanteEsquerda1 = newCircle(ataEsq1, 15, 5, "rosa.png", moveAtacante, 0.2, 0, init_jogador_data(ataEsq1, LEFT));
    atacanteEsquerda2 = newCircle(ataEsq2, 15, 5, "rosa.png", moveAtacante, 0.2, 0, init_jogador_data(ataEsq2, LEFT));
    goleiroDireita = newCircle(golDir, 11, 5, "branco.png", moveGoleiro, 0.2, 2, init_jogador_data(golDir, RIGHT));
    zagueiroDireita1= newCircle(defeDir1, 15, 5, "branco.png", moveDefensor, 0.2, 0, init_jogador_data(defeDir1, RIGHT));
    zagueiroDireita2= newCircle(defeDir2, 15, 5, "branco.png", moveDefensor, 0.2, 0, init_jogador_data(defeDir2, RIGHT));
    zagueiroDireita3= newCircle(defeDir3, 15, 5, "branco.png", moveDefensor, 0.2, 0, init_jogador_data(defeDir3, RIGHT));
    atacanteDireita1= newCircle(ataDir1, 15, 5, "branco.png", moveAtacante, 0.2, 0, init_jogador_data(ataDir1, RIGHT));
    atacanteDireita2= newCircle(ataDir2, 15, 5, "branco.png", moveAtacante, 0.2, 0, init_jogador_data(ataDir2, RIGHT));
    pthread_create(&thread_id, NULL, isTheBallStuck, NULL);
}

void * isTheBallStuck(void *arg) {
    while (1) {
        cpVect pos1 = cpBodyGetPosition(ballBody);
        sleep(1);
        cpVect pos2 = cpBodyGetPosition(ballBody);
        if (fabs(pos1.x - pos2.x) < 1 && fabs(pos1.y - pos2.y) < 1) {
            resetPositionPlayers(); 
        }
    }
}

void resetPositionPlayers () {
    cpBodySetPosition(goleiroEsquerda, golEsq);
    cpBodySetPosition(zagueiroEsquerda1, defeEsq1);
    cpBodySetPosition(zagueiroEsquerda2, defeEsq2);
    cpBodySetPosition(zagueiroEsquerda3, defeEsq3);
    cpBodySetPosition(atacanteEsquerda1, ataEsq1);
    cpBodySetPosition(atacanteEsquerda2, ataEsq2);
    cpBodySetPosition(goleiroDireita, golDir);
    cpBodySetPosition(zagueiroDireita1, defeDir1);
    cpBodySetPosition(zagueiroDireita2, defeDir2);
    cpBodySetPosition(zagueiroDireita3, defeDir3);
    cpBodySetPosition(atacanteDireita1, ataDir1);
    cpBodySetPosition(atacanteDireita2, ataDir2);

    cpBodySetVelocity(ballBody, cpv(0,0));
    cpBodySetPosition(ballBody, centroDoCampo);
}

void checkAndApplyBoundaryImpulse(cpBody* body, void* data) {
    UserData* ud = (UserData*)data;
    cpFloat raio = ud->radius;

    cpVect pos = cpBodyGetPosition(body);
    cpVect vel = cpBodyGetVelocity(body);

    if (pos.x - raio <= 0) {
        cpVect impulse = cpv(50.0, 0.0); 
        cpBodyApplyImpulseAtLocalPoint(body, impulse, cpvzero);
    }
    if (pos.x + raio >= LARGURA_JAN) { 
        cpVect impulse = cpv(-50.0, 0.0); 
        cpBodyApplyImpulseAtLocalPoint(body, impulse, cpvzero);
    }
    if (pos.y - raio <= 0) {  
        cpVect impulse = cpv(0.0, 50.0); 
        cpBodyApplyImpulseAtLocalPoint(body, impulse, cpvzero); 
    }
    if (pos.y + raio >= ALTURA_JAN) {
        cpVect impulse = cpv(0.0, -50.0);
        cpBodyApplyImpulseAtLocalPoint(body, impulse, cpvzero); 
    }
}

void moveGoleiro(cpBody* body, void* data){
    UserData* ud = (UserData*)data;
    jogador_data j = ud->jogadorData;

    checkAndApplyBoundaryImpulse(body, data);

    cpVect vel = cpBodyGetVelocity(body);
    vel = cpvclamp(vel, 10);
    cpBodySetVelocity(body, vel);

    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvmult(cpvnormalize(cpvadd(ballPos,pos)),20);

    cpVect posicao_goleira;
    int valor_grande_area;
    if (j.lado == LEFT) {
        posicao_goleira = golEsq;
        valor_grande_area = 189;
    } else {
        posicao_goleira = golDir;
        valor_grande_area = 835;
    }

    bool in_grande_area = (ballPos.x < valor_grande_area && j.lado == LEFT) || (ballPos.x > valor_grande_area && j.lado == RIGHT);
    bool in_valid_y_range = (ballPos.y < 533 && ballPos.y > 180);

    if (in_grande_area && in_valid_y_range) {
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    } else {
        cpBodyApplyImpulseAtWorldPoint(body, cpvadd(posicao_goleira, pos), posicao_goleira);
        if (fabs(robotPos.x - posicao_goleira.x) < 0.1 && fabs(robotPos.y - posicao_goleira.y) < 0.1) {
            cpBodySetVelocity(body, cpv(0,0));
        }
    }
}

void moveAtacante(cpBody* body, void* data) {
    UserData* ud = (UserData*)data;
    jogador_data j = ud->jogadorData;
    cpVect jogador_posicao = j.pos_inicial;

    checkAndApplyBoundaryImpulse(body, data);

    cpVect vel = cpvclamp(cpBodyGetVelocity(body), 30);
    cpBodySetVelocity(body, vel);

    cpVect ballPos = cpBodyGetPosition(ballBody);
    cpVect robotPos = cpBodyGetPosition(body);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;

    cpVect delta = cpvmult(cpvnormalize(cpvadd(ballPos, cpvneg(robotPos))), 20);

    cpVect posicao_goleira;
    cpVect posicaoOutroAtacante;

    if (j.lado == LEFT) {
        posicao_goleira = golDir;
        posicaoOutroAtacante = (body == atacanteEsquerda1) ? obterPosicaoAtacante(j.lado, 1) : obterPosicaoAtacante(j.lado, 0);
    } else {
        posicao_goleira = golEsq;
        posicaoOutroAtacante = (body == atacanteDireita1) ? obterPosicaoAtacante(j.lado, 1) : obterPosicaoAtacante(j.lado, 0);
    }

    cpFloat distanciaParaBola = cpvdist(robotPos, ballPos);
    cpFloat distanciaParaGol = cpvdist(robotPos, posicao_goleira);
    cpFloat distanciaOutroAtacanteParaGol = cpvdist(posicaoOutroAtacante, posicao_goleira);

    if (distanciaParaGol > distanciaOutroAtacanteParaGol && distanciaParaBola <28) {
        aplicarImpulsoNaBola(posicaoOutroAtacante, ballPos);
    } else if (distanciaParaBola < 32) {
        aplicarImpulsoNaBola(posicao_goleira, ballPos);
    }

    bool isLeft = (j.lado == LEFT);
    cpBool isCloser = cpvdist(posicaoOutroAtacante, ballPos) > distanciaParaBola;

    if ((isLeft && ballPos.x > 430) || (!isLeft && ballPos.x < 596)) {
        if (isCloser) {
            cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
        }
    } else {
        cpVect deltaIni = cpvadd(jogador_posicao, pos);
        cpBodyApplyImpulseAtWorldPoint(body, deltaIni, jogador_posicao);
        if (fabs(robotPos.x - jogador_posicao.x) < 0.1 && fabs(robotPos.y - jogador_posicao.y) < 0.1) {
            cpBodySetVelocity(body, cpv(0, 0));
        }
    }
}

cpVect obterPosicaoAtacante(lado lado, int atacante) {
    switch (lado) {
        case RIGHT: return (atacante == 0) ? cpBodyGetPosition(atacanteDireita1) : cpBodyGetPosition(atacanteDireita2);
        case LEFT: return (atacante == 0) ? cpBodyGetPosition(atacanteEsquerda1) : cpBodyGetPosition(atacanteEsquerda2);
        default: return cpvzero;
    }
}

void aplicarImpulsoNaBola(cpVect mira, cpVect ballPos) {
    srand(time(NULL));
    int sorteio = rand() % 51;
    mira.x -= sorteio;
    cpVect chute = cpvmult(cpvnormalize(cpvsub(mira, ballPos)), 7);
    cpBodyApplyImpulseAtWorldPoint(ballBody, chute, ballPos);
}

void moveDefensor (cpBody* body, void* data){
    UserData* ud = (UserData*)data;
    jogador_data j = ud->jogadorData; 

    cpVect jogador_posicao = j.pos_inicial;

    checkAndApplyBoundaryImpulse(body, data);

    cpVect vel = cpBodyGetVelocity(body);
    vel = cpvclamp(vel, 30);
    cpBodySetVelocity(body, vel);
    
    // Obtém a posição do robô e da bola...
    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos,pos);

    if (cpvdist(robotPos, ballPos) < 25) { // os defensores estavam muito bons, reduzimos o raio de ação
        srand(time(NULL));
        int sorteio = rand() % 2;
        aplicarImpulsoNaBola(obterPosicaoAtacante(j.lado, sorteio), ballPos);
    }

    //chega perto da bola?
    int valores[2];
    if (j.pos_inicial.y == 180) {
        valores[0] = 0;
        valores[1] = 237;
    } else if (j.pos_inicial.y == 356) {
        valores[0] = 237;
        valores[1] = 474;
    } else {
        valores[0] = 474;
        valores[1] = 712;
    }

    bool isLeft = (j.lado == LEFT);
    bool isBallInZone = (ballPos.y > valores[0] && ballPos.y < valores[1]);
    bool isLeftZone = (ballPos.x <= 512);
    bool isRightZone = (ballPos.x > 512);

    if ((isLeft && isLeftZone && isBallInZone) || (!isLeft && isRightZone && isBallInZone)) {
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    } else {
        cpVect deltaIni = cpvadd(jogador_posicao, pos);
        cpBodyApplyImpulseAtWorldPoint(body, deltaIni, jogador_posicao);
        if (fabs(robotPos.x - jogador_posicao.x) < 0.1 && fabs(robotPos.y - jogador_posicao.y) < 0.1) {
            cpBodySetVelocity(body, cpv(0, 0));
        }
    }
}

void moveBola(cpBody* body, void* data){
    cpVect velBola = cpvclamp(cpBodyGetVelocity(body), 55);
    cpBodySetVelocity(body, velBola);
    cpVect ballPos  = cpBodyGetPosition(body);
    if(ballPos.x < 45 && ballPos.y > 326 && ballPos.y < 386){
        score2++;
        resetPositionPlayers();
    }
    if(ballPos.x > 978 && ballPos.y > 326 && ballPos.y < 386){
        score1++;
        resetPositionPlayers();
    }
    isTheGameOver();
}

void isTheGameOver () {
    if((score1 >= 3 && (score1-score2)>2) || (score2 >= 3 &&(score2-score1)>2)){
        gameOver = 1;
    }
}

void freeCM()
{
    printf("Cleaning up!\n");
    UserData* ud = cpBodyGetUserData(ballBody);
    cpShapeFree(ud->shape);
    cpBodyFree(ballBody);

    ud = cpBodyGetUserData(goleiroDireita);
    cpShapeFree(ud->shape);
    cpBodyFree(goleiroDireita);
    free(ud);
    ud = cpBodyGetUserData(zagueiroDireita1);
    cpShapeFree(ud->shape);
    cpBodyFree(zagueiroDireita1);
    free(ud);
    ud = cpBodyGetUserData(zagueiroDireita2);
    cpShapeFree(ud->shape);
    cpBodyFree(zagueiroDireita2);
    free(ud);
    ud = cpBodyGetUserData(zagueiroDireita3);
    cpShapeFree(ud->shape);
    cpBodyFree(zagueiroDireita3);
    free(ud);
    ud = cpBodyGetUserData(atacanteDireita1);
    cpShapeFree(ud->shape);
    cpBodyFree(atacanteDireita1);
    free(ud);
    ud = cpBodyGetUserData(atacanteDireita2);
    cpShapeFree(ud->shape);
    cpBodyFree(atacanteDireita2);
    free(ud);

    ud = cpBodyGetUserData(goleiroEsquerda);
    cpShapeFree(ud->shape);
    cpBodyFree(goleiroDireita);
    free(ud);
    ud = cpBodyGetUserData(zagueiroEsquerda1);
    cpShapeFree(ud->shape);
    cpBodyFree(zagueiroEsquerda1);
    free(ud);
    ud = cpBodyGetUserData(zagueiroEsquerda2);
    cpShapeFree(ud->shape);
    cpBodyFree(zagueiroEsquerda2);
    free(ud);
    ud = cpBodyGetUserData(zagueiroEsquerda3);
    cpShapeFree(ud->shape);
    cpBodyFree(zagueiroEsquerda3);
    free(ud);
    ud = cpBodyGetUserData(atacanteEsquerda1);
    cpShapeFree(ud->shape);
    cpBodyFree(atacanteEsquerda1);
    free(ud);
    ud = cpBodyGetUserData(atacanteEsquerda2);
    cpShapeFree(ud->shape);
    cpBodyFree(atacanteEsquerda2);
    free(ud);

    cpShapeFree(leftWall);
    cpShapeFree(rightWall);
    cpShapeFree(bottomWall);
    cpShapeFree(topWall);
    cpShapeFree(goleiraDireitaCima);
    cpShapeFree(goleiraDireitaBaixo);
    cpShapeFree(goleiraEsquerdaCima);
    cpShapeFree(goleiraEsquerdaBaixo);

    cpSpaceFree(space);
}

void restartCM()
{
    gameOver = 0;
    score1 = 0;
    score2 = 0;
    resetPositionPlayers();
    
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
cpBody* newCircle(cpVect pos, cpFloat radius, cpFloat mass, char* img, bodyMotionFunc func, cpFloat fric, cpFloat elast, jogador_data jogadorData)
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
    newUserData->jogadorData = jogadorData;
    cpBodySetUserData(newBody, newUserData);
    printf("newCircle: loaded img %s\n", img);
    return newBody;
}