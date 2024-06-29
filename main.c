#include <math.h>
#include <chipmunk.h>
#include <SOIL.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

// Rotinas para acesso da OpenGL
#include "opengl.h"

void isTheGameOver ();
void *isTheBallStuck(void *arg);
void resetPositionPlayers ();

// Funções para movimentação de objetos
void moveBola(cpBody* body, void* data);
void moveGoleiroDireita(cpBody* body, void* data);
void moveZagueiroDireita1(cpBody* body, void* data);
void moveZagueiroDireita2(cpBody* body, void* data);
void moveZagueiroDireita3(cpBody* body, void* data);
void moveAtacanteDireita1(cpBody* body, void* data);
void moveAtacanteDireita2(cpBody* body, void* data);

void moveGoleiroEsquerda(cpBody* body, void* data);
void moveZagueiroEsquerda1(cpBody* body, void* data);
void moveZagueiroEsquerda2(cpBody* body, void* data);
void moveZagueiroEsquerda3(cpBody* body, void* data);
void moveAtacanteEsquerda1(cpBody* body, void* data);
void moveAtacanteEsquerda2(cpBody* body, void* data);

// Prototipos
void initCM();
void freeCM();
void restartCM();
cpShape* newLine(cpVect inicio, cpVect fim, cpFloat fric, cpFloat elast);
cpBody* newCircle(cpVect pos, cpFloat radius, cpFloat mass, char* img, bodyMotionFunc func, cpFloat fric, cpFloat elast);

// Velocidade dos jogadores
int velocidadeJogador = 10;

int velocidadeGoleiros = 10;

// Limita o impulso aplicado nos jogadores
int limiteImpulso = 20;

// Limita impulso aplicado na bola
int limiteImpulsoBola = 40;

// Distancia que o jogador chega da bola para chutar
int DISTANCIA_PARA_CHUTAR = 28;
int TEMPO_DE_JOGO = 110;

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
cpShape* leftWall, *rightWall, *topWall, *bottomWall, *travessaoD1, *travessaoD2, *travessaoE1, *travessaoE2;

// A bola
cpBody* ballBody;

// Jogadores time 1
cpBody* goleiroDireita, *zagueiroDireita1, *zagueiroDireita2, *zagueiroDireita3, *atacanteDireita1, *atacanteDireita2;

// Jogadores time 2
cpBody* goleiroEsquerda, *zagueiroEsquerda1, *zagueiroEsquerda2, *zagueiroEsquerda3, *atacanteEsquerda1, *atacanteEsquerda2;

//Variaveis global de coordenadas time 1
cpVect golEsq, zagEsq1, zagEsq2, zagEsq3, ataEsq1, ataEsq2, goleiraEsq;

//Variaveis global de coordenadas time 2
cpVect golDir, zagDir1, zagDir2, zagDir3, ataDir1, ataDir2, goleiraDir;

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
    // campo : 1024 x 700
    golEsq = cpv(120,356);
    zagEsq1 = cpv(256,180);
    zagEsq2 = cpv(324,356);
    zagEsq3 = cpv(256,534);
    ataEsq1 = cpv(440,317);
    ataEsq2 = cpv(440,395);
    goleiraEsq = cpv(35, 356);

    golDir = cpv(924,356);
    zagDir1 = cpv(770,180);
    zagDir2 = cpv(700,356);
    zagDir3 = cpv(770,534);
    ataDir1 = cpv(585,317);
    ataDir2 = cpv(586,396);
    goleiraDir = cpv(987, 356);

    centroDoCampo = cpv(512,350);

    //Goleira direita
    travessaoD1 = newLine(cpv(976,325), cpv(LARGURA_JAN, 326), 0, 1.0);
    travessaoD2 = newLine(cpv(976,386), cpv(LARGURA_JAN, 386), 0, 1.0);

    //Goleira esquerda
    travessaoD1 = newLine(cpv(0,325), cpv(47, 326), 0, 1.0);
    travessaoD2 = newLine(cpv(0,386), cpv(77, 386), 0, 1.0);

    //Bola
    ballBody = newCircle(centroDoCampo, 8, 1, "small_football.png", moveBola, 0.2, 1);


    // Jogadores time 1
    goleiroEsquerda = newCircle(golEsq, 10, 5, "interGol.png", moveGoleiroEsquerda, 0.2, 2);
    zagueiroEsquerda1 = newCircle(zagEsq1, 15, 5, "Inter.png", moveZagueiroEsquerda1, 0.2, 0);
    zagueiroEsquerda2 = newCircle(zagEsq2, 15, 5, "Inter.png", moveZagueiroEsquerda2, 0.2, 0);
    zagueiroEsquerda3 = newCircle(zagEsq3, 15, 5, "Inter.png", moveZagueiroEsquerda3, 0.2, 0);
    atacanteEsquerda1 = newCircle(ataEsq1, 15, 5, "Inter.png", moveAtacanteEsquerda1, 0.2, 0);
    atacanteEsquerda2 = newCircle(ataEsq2, 15, 5, "Inter.png", moveAtacanteEsquerda2, 0.2, 0);

    // Jogadores time 2
    goleiroDireita = newCircle(golDir, 10, 5, "gremioGol.png", moveGoleiroDireita, 0.2, 2);
    zagueiroDireita1= newCircle(zagDir1, 15, 5, "Gremio.png", moveZagueiroDireita1, 0.2, 0);
    zagueiroDireita2= newCircle(zagDir2, 15, 5, "Gremio.png", moveZagueiroDireita2, 0.2, 0);
    zagueiroDireita3= newCircle(zagDir3, 15, 5, "Gremio.png", moveZagueiroDireita3, 0.2, 0);
    atacanteDireita1= newCircle(ataDir1, 15, 5, "Gremio.png", moveAtacanteDireita1, 0.2, 0);
    atacanteDireita2= newCircle(ataDir2, 15, 5, "Gremio.png", moveAtacanteDireita2, 0.2, 0);

    pthread_create(&thread_id, NULL, isTheBallStuck, NULL);
}

int forcaChute(){
    int random = 5 + (numeroAleatorio()%6);
    return random;
}

int numeroAleatorio() {
    //Retorna um numero aleatorio de 0 a 10
    srand(time(0));
    return rand() % 11;
}

void * isTheBallStuck(void *arg) {
    while (1) {
        cpVect pos1 = cpBodyGetPosition(ballBody);
        sleep(2);
        cpVect pos2 = cpBodyGetPosition(ballBody);
        if (fabs(pos1.x - pos2.x) < 1 && fabs(pos1.y - pos2.y) < 1) {
            resetPositionPlayers(); 
        }
    }
}

void resetPositionPlayers () {
    //Time 1
    cpBodySetPosition(goleiroEsquerda, golEsq);
    cpBodySetPosition(zagueiroEsquerda1, zagEsq1);
    cpBodySetPosition(zagueiroEsquerda2, zagEsq2);
    cpBodySetPosition(zagueiroEsquerda3, zagEsq3);
    cpBodySetPosition(atacanteEsquerda1, ataEsq1);
    cpBodySetPosition(atacanteEsquerda2, ataEsq2);

    //Time 2
    cpBodySetPosition(goleiroDireita, golDir);
    cpBodySetPosition(zagueiroDireita1, zagDir1);
    cpBodySetPosition(zagueiroDireita2, zagDir2);
    cpBodySetPosition(zagueiroDireita3, zagDir3);
    cpBodySetPosition(atacanteDireita1, ataDir1);
    cpBodySetPosition(atacanteDireita2, ataDir2);

    cpBodySetVelocity(ballBody, cpv(0,0));
    cpBodySetPosition(ballBody, centroDoCampo);
}

void moveGoleiroEsquerda(cpBody* body, void* data){
    cpVect vel = cpBodyGetVelocity(body);
    vel = cpvclamp(vel, (velocidadeGoleiros));
    cpBodySetVelocity(body, vel);

    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos,pos);
    delta = cpvmult(cpvnormalize(delta),limiteImpulso);

    if(ballPos.x < 189 && ballPos.y < 533 && ballPos.y > 180){
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    } else{
        cpVect deltaIni = cpvadd(golEsq,pos);
        cpBodyApplyImpulseAtWorldPoint(body, deltaIni, golEsq);
        if (fabs(robotPos.x - golEsq.x) < 0.1 && fabs(robotPos.y - golEsq.y) < 0.1 ){
             cpBodySetVelocity(body, cpv(0,0));
        }
    }
}

void moveZagueiroEsquerda1(cpBody* body, void* data){
    cpVect vel = cpBodyGetVelocity(body);
    vel = cpvclamp(vel, (velocidadeJogador));
    cpBodySetVelocity(body, vel);

    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos,pos);
    delta = cpvmult(cpvnormalize(delta), limiteImpulso);
    
    cpFloat distance = cpvdist(robotPos, ballPos);

    // Passa a bola para um dos atacantes
    if (distance < DISTANCIA_PARA_CHUTAR) {
        cpVect posicaoAta;
        int random = numeroAleatorio() % 2;
        if(random){
            posicaoAta = cpBodyGetPosition(atacanteEsquerda2);
        }else{
            posicaoAta = cpBodyGetPosition(atacanteEsquerda1);
        }
        cpVect chute = cpvmult(cpvnormalize(cpvsub(posicaoAta, ballPos)), forcaChute());
        cpBodyApplyImpulseAtWorldPoint(ballBody, chute, ballPos);
    }

    if (ballPos.x < 509 && ballPos.y < 236){
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    } else{
        cpVect deltaIni = cpvadd(zagEsq1,pos);
        cpBodyApplyImpulseAtWorldPoint(body, deltaIni, zagEsq1);
        if (fabs(robotPos.x - zagEsq1.x) < 0.1 && fabs(robotPos.y - zagEsq1.y) < 0.1 ){
             cpBodySetVelocity(body, cpv(0,0));
        }
    }
}

void moveZagueiroEsquerda2(cpBody* body, void* data){
    cpVect vel = cpBodyGetVelocity(body);
    vel = cpvclamp(vel, (velocidadeJogador));
    cpBodySetVelocity(body, vel);

    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos,pos);
    delta = cpvmult(cpvnormalize(delta), limiteImpulso);

    cpFloat distance = cpvdist(robotPos, ballPos);

    if (distance < DISTANCIA_PARA_CHUTAR) {
        cpVect posicaoAta;
        int random = numeroAleatorio() % 2;
        if(random){
            posicaoAta = cpBodyGetPosition(atacanteEsquerda2);
        }else{
            posicaoAta = cpBodyGetPosition(atacanteEsquerda1);
        }
        posicaoAta.x = posicaoAta.x;
        cpVect chute = cpvmult(cpvnormalize(cpvsub(posicaoAta, ballPos)), forcaChute());
        cpBodyApplyImpulseAtWorldPoint(ballBody, chute, ballPos);
    }
    
    if ( ballPos.x < 509 && ballPos.y > 236 && ballPos.y < 475){
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    } else{
        cpVect deltaIni = cpvadd(zagEsq2,pos);
        cpBodyApplyImpulseAtWorldPoint(body, deltaIni, zagEsq2);
        if (fabs(robotPos.x - zagEsq2.x) < 0.1 && fabs(robotPos.y - zagEsq2.y) < 0.1 ){
             cpBodySetVelocity(body, cpv(0,0));
        }
    }
}

void moveZagueiroEsquerda3(cpBody* body, void* data){
    cpVect vel = cpBodyGetVelocity(body);
    vel = cpvclamp(vel, (velocidadeJogador));
    cpBodySetVelocity(body, vel);

    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos,pos);
    delta = cpvmult(cpvnormalize(delta), limiteImpulso);

    cpFloat distance = cpvdist(robotPos, ballPos);

    if (distance < DISTANCIA_PARA_CHUTAR) {
        cpVect posicaoAta;
        int random = numeroAleatorio() % 2;
        if(random){
            posicaoAta = cpBodyGetPosition(atacanteEsquerda2);
        }else{
            posicaoAta = cpBodyGetPosition(atacanteEsquerda1);
        }
        posicaoAta.x = posicaoAta.x;
        cpVect chute = cpvmult(cpvnormalize(cpvsub(posicaoAta, ballPos)), forcaChute());
        cpBodyApplyImpulseAtWorldPoint(ballBody, chute, ballPos);
    }

    if ( ballPos.x < 509 && ballPos.y > 475){
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    } else{
        cpVect deltaIni = cpvadd(zagEsq3, pos);
        cpBodyApplyImpulseAtWorldPoint(body, deltaIni, zagEsq3);
        if (fabs(robotPos.x - zagEsq3.x) < 0.1 && fabs(robotPos.y - zagEsq3.y) < 0.1 ){
             cpBodySetVelocity(body, cpv(0,0));
        }
    }
}

void moveAtacanteEsquerda1(cpBody* body, void* data) {
    cpVect vel = cpBodyGetVelocity(body);
    vel = cpvclamp(vel, velocidadeJogador);
    cpBodySetVelocity(body, vel);

    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos, pos);
    delta = cpvmult(cpvnormalize(delta), limiteImpulso);

    cpFloat distance = cpvdist(robotPos, ballPos);

    cpVect posicaoOutroAta = cpBodyGetPosition(atacanteEsquerda2);
    cpFloat distanceAtaPGol = cpvdist(posicaoOutroAta, goleiraDir);
    cpFloat distanceAtaAtual = cpvdist(cpBodyGetPosition(body), goleiraDir);
    
    if (distanceAtaAtual > distanceAtaPGol && distance < DISTANCIA_PARA_CHUTAR) {
        cpVect chute = cpvmult(cpvnormalize(cpvsub(posicaoOutroAta, ballPos)), forcaChute());
        cpBodyApplyImpulseAtWorldPoint(ballBody, chute, ballPos);
    } else if (distance < DISTANCIA_PARA_CHUTAR) {
        cpVect chute = cpvmult(cpvnormalize(cpvsub(goleiraDir, ballPos)), forcaChute());
        cpBodyApplyImpulseAtWorldPoint(ballBody, chute, ballPos);
    }

    if (ballPos.x > 430 && cpvdist(posicaoOutroAta, ballPos) > cpvdist(cpBodyGetPosition(body), ballPos)) {
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    } else {
        cpBodySetVelocity(body, cpv(0,0));
    }
}

void moveAtacanteEsquerda2(cpBody* body, void* data){
    cpVect vel = cpBodyGetVelocity(body);
    vel = cpvclamp(vel, (velocidadeJogador));
    cpBodySetVelocity(body, vel);

    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos,pos);
    delta = cpvmult(cpvnormalize(delta), limiteImpulso);

    cpFloat distance = cpvdist(robotPos, ballPos);

    cpVect posicaoOutroAta = cpBodyGetPosition(atacanteEsquerda1);
    cpFloat distanceAtaPGol = cpvdist(posicaoOutroAta, goleiraDir);
    cpFloat distanceAtaAtual = cpvdist(cpBodyGetPosition(body), goleiraDir);
    
    if (distanceAtaAtual > distanceAtaPGol && distance < DISTANCIA_PARA_CHUTAR) {
        cpVect chute = cpvmult(cpvnormalize(cpvsub(posicaoOutroAta, ballPos)), forcaChute());
        cpBodyApplyImpulseAtWorldPoint(ballBody, chute, ballPos);
    } else if (distance < DISTANCIA_PARA_CHUTAR) {
        cpVect chute = cpvmult(cpvnormalize(cpvsub(goleiraDir, ballPos)), forcaChute());
        cpBodyApplyImpulseAtWorldPoint(ballBody, chute, ballPos);
    }
    if (ballPos.x > 430 && cpvdist(posicaoOutroAta, ballPos) > cpvdist(cpBodyGetPosition(body), ballPos)){
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    }else{   
        cpBodySetVelocity(body, cpv(0,0));
    }
}

void moveGoleiroDireita(cpBody* body, void* data){
    cpVect vel = cpBodyGetVelocity(body);
    vel = cpvclamp(vel, (velocidadeGoleiros));
    cpBodySetVelocity(body, vel);

    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos,pos);
    delta = cpvmult(cpvnormalize(delta), limiteImpulso);

    if(ballPos.x > 835 && robotPos.y < 533 && robotPos.y > 180){
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    } else{
        cpVect deltaIni = cpvadd(golDir, pos);
        cpBodyApplyImpulseAtWorldPoint(body, deltaIni, golDir);
        if (fabs(robotPos.x - golDir.x) < 0.1 && fabs(robotPos.y - golDir.y) < 0.1 ){
             cpBodySetVelocity(body, cpv(0,0));
        }
    }
}

void moveZagueiroDireita1(cpBody* body, void* data){
    cpVect vel = cpBodyGetVelocity(body);
    vel = cpvclamp(vel, (velocidadeJogador));
    cpBodySetVelocity(body, vel);

    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos,pos);
    delta = cpvmult(cpvnormalize(delta), limiteImpulso);

    cpFloat distance = cpvdist(robotPos, ballPos);
    if (distance < DISTANCIA_PARA_CHUTAR) {
        cpVect posicaoAta;
        int random = numeroAleatorio() % 2;
        if(random){
            posicaoAta = cpBodyGetPosition(atacanteDireita2);
        }else{
            posicaoAta = cpBodyGetPosition(atacanteDireita1);
        }
        posicaoAta.x = posicaoAta.x - 50;
        cpVect chute = cpvmult(cpvnormalize(cpvsub(posicaoAta, ballPos)), forcaChute());
        cpBodyApplyImpulseAtWorldPoint(ballBody, chute, ballPos);
    }

    if ( ballPos.x > 512 && ballPos.y < 236){
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    } else{
        cpVect deltaIni = cpvadd(zagDir1, pos);
        cpBodyApplyImpulseAtWorldPoint(body, deltaIni, zagDir1);
        if (fabs(robotPos.x - zagDir1.x) < 0.1 && fabs(robotPos.y - zagDir1.y) < 0.1 ){
             cpBodySetVelocity(body, cpv(0,0));
        }
    }
}

void moveZagueiroDireita2(cpBody* body, void* data){
    cpVect vel = cpBodyGetVelocity(body);
    vel = cpvclamp(vel, (velocidadeJogador));
    cpBodySetVelocity(body, vel);

    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos,pos);
    delta = cpvmult(cpvnormalize(delta), limiteImpulso);

    cpFloat distance = cpvdist(robotPos, ballPos);

    if (distance < DISTANCIA_PARA_CHUTAR) {
        cpVect posicaoAta;
        int random = numeroAleatorio() % 2;
        if(random){
            posicaoAta = cpBodyGetPosition(atacanteDireita2);
        }else{
            posicaoAta = cpBodyGetPosition(atacanteDireita1);
        }
        posicaoAta.x = posicaoAta.x - 50;
        cpVect chute = cpvmult(cpvnormalize(cpvsub(posicaoAta, ballPos)), forcaChute());
        cpBodyApplyImpulseAtWorldPoint(ballBody, chute, ballPos);
    }

    if ( ballPos.x > 512 && ballPos.y > 236 && ballPos.y < 475){
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    } else{
        cpVect deltaIni = cpvadd(zagDir2,pos);
        cpBodyApplyImpulseAtWorldPoint(body, deltaIni, zagDir2);
        if (fabs(robotPos.x - zagDir2.x) < 0.1 && fabs(robotPos.y - zagDir2.y) < 0.1 ){
             cpBodySetVelocity(body, cpv(0,0));
        }
    }
}

void moveZagueiroDireita3(cpBody* body, void* data){
    cpVect vel = cpBodyGetVelocity(body);
    vel = cpvclamp(vel, (velocidadeJogador));
    cpBodySetVelocity(body, vel);

    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos,pos);
    delta = cpvmult(cpvnormalize(delta), limiteImpulso);
    
    cpFloat distance = cpvdist(robotPos, ballPos);
    if (distance < DISTANCIA_PARA_CHUTAR) {
        cpVect posicaoAta;
        int random = numeroAleatorio() % 2;
        if(random){
            posicaoAta = cpBodyGetPosition(atacanteDireita2);
        }else{
            posicaoAta = cpBodyGetPosition(atacanteDireita1);
        }
        posicaoAta.x = posicaoAta.x - 50;
        cpVect chute = cpvmult(cpvnormalize(cpvsub(posicaoAta, ballPos)), forcaChute());
        cpBodyApplyImpulseAtWorldPoint(ballBody, chute, ballPos);
    }

    if ( ballPos.x > 512 && ballPos.y > 475){
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    } else{
        cpVect deltaIni = cpvadd(zagDir3,pos);
        cpBodyApplyImpulseAtWorldPoint(body, deltaIni, zagDir3);
        if (fabs(robotPos.x - zagDir3.x) < 0.1 && fabs(robotPos.y - zagDir3.y) < 0.1 ){
             cpBodySetVelocity(body, cpv(0,0));
        }
    }
}

void moveAtacanteDireita1(cpBody* body, void* data){
    cpVect vel = cpBodyGetVelocity(body);
    vel = cpvclamp(vel, (velocidadeJogador));
    cpBodySetVelocity(body, vel);

    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos,pos);
    delta = cpvmult(cpvnormalize(delta), limiteImpulso);
    
    cpFloat distance = cpvdist(robotPos, ballPos);

    cpFloat distanceAtaPGol = cpvdist(cpBodyGetPosition(atacanteDireita2), goleiraEsq);
    cpFloat distanceAtaAtual = cpvdist(cpBodyGetPosition(body), goleiraEsq);

    if (distance < DISTANCIA_PARA_CHUTAR) {
        if(distanceAtaPGol < distanceAtaAtual ){
            cpVect chute = cpvmult(cpvnormalize(cpvsub(cpBodyGetPosition(atacanteDireita2), ballPos)), forcaChute());
            cpBodyApplyImpulseAtWorldPoint(ballBody, chute, ballPos);
        }else{
            cpVect chute = cpvmult(cpvnormalize(cpvsub(goleiraEsq, ballPos)), forcaChute());
            cpBodyApplyImpulseAtWorldPoint(ballBody, chute, ballPos);
        }
    }

    if (ballPos.x < 596 && cpvdist(cpBodyGetPosition(atacanteDireita2), ballPos) > cpvdist(cpBodyGetPosition(body), ballPos)){
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    }else{
        cpBodySetVelocity(body, cpv(0,0));
    }
}

void moveAtacanteDireita2(cpBody* body, void* data){
    cpVect vel = cpBodyGetVelocity(body);
    vel = cpvclamp(vel, (velocidadeJogador));
    cpBodySetVelocity(body, vel);

    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);

    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta = cpvadd(ballPos,pos);
    delta = cpvmult(cpvnormalize(delta), limiteImpulso);

    cpFloat distance = cpvdist(robotPos, ballPos);

    cpFloat distanceAtaPGol = cpvdist(cpBodyGetPosition(atacanteDireita1), goleiraEsq);
    cpFloat distanceAtaAtual = cpvdist(cpBodyGetPosition(body), goleiraEsq);

    if (distance < DISTANCIA_PARA_CHUTAR) {
        if(distanceAtaPGol < distanceAtaAtual ){
            cpVect chute = cpvmult(cpvnormalize(cpvsub(cpBodyGetPosition(atacanteDireita1), ballPos)), forcaChute());
            cpBodyApplyImpulseAtWorldPoint(ballBody, chute, ballPos);
        }else{
            cpVect chute = cpvmult(cpvnormalize(cpvsub(goleiraEsq, ballPos)), forcaChute());
            cpBodyApplyImpulseAtWorldPoint(ballBody, chute, ballPos);
        }
    }

    if (ballPos.x < 596 && cpvdist(cpBodyGetPosition(atacanteDireita1), ballPos) > cpvdist(cpBodyGetPosition(body), ballPos)){
        cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    }else{
        cpBodySetVelocity(body, cpv(0,0));
    }
}

void moveBola(cpBody* body, void* data){
    //tiramos o impulso randomico pois os jogadores q dao o impulso, e nao a bola por si só

    //para a bola não ir tão rápido, limitamos até 70
    cpVect velBola = cpvclamp(cpBodyGetVelocity(body), 70);
    cpBodySetVelocity(body, velBola);

    //verificamos se a bola está dentro do gol
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
    if((score1 >= 3 && (score1-score2)>2) || (score2 >= 3 &&(score1-score2)>2)){
        gameOver = 1;
    }
}

void freeCM()
{
    // Libera memória ocupada por cada corpo, forma e ambiente
    // Acrescente mais linhas caso necessário

    printf("Cleaning up!\n");
    UserData* ud = cpBodyGetUserData(ballBody);
    cpShapeFree(ud->shape);
    cpBodyFree(ballBody);

    //Liberando o time esquerda
    ud = cpBodyGetUserData(goleiroEsquerda);
    cpShapeFree(ud->shape);
    cpBodyFree(goleiroEsquerda);

    ud = cpBodyGetUserData(zagueiroEsquerda1);
    cpShapeFree(ud->shape);
    cpBodyFree(zagueiroEsquerda1);

    ud = cpBodyGetUserData(zagueiroEsquerda2);
    cpShapeFree(ud->shape);
    cpBodyFree(zagueiroEsquerda2);

    ud = cpBodyGetUserData(zagueiroEsquerda3);
    cpShapeFree(ud->shape);
    cpBodyFree(zagueiroEsquerda3);

    ud = cpBodyGetUserData(atacanteEsquerda1);
    cpShapeFree(ud->shape);
    cpBodyFree(atacanteEsquerda1);

    ud = cpBodyGetUserData(atacanteEsquerda2);
    cpShapeFree(ud->shape);
    cpBodyFree(atacanteEsquerda2);

    //Liberando o time direito
    ud = cpBodyGetUserData(goleiroDireita);
    cpShapeFree(ud->shape);
    cpBodyFree(goleiroDireita);

    ud = cpBodyGetUserData(zagueiroDireita1);
    cpShapeFree(ud->shape);
    cpBodyFree(zagueiroDireita2);

    ud = cpBodyGetUserData(zagueiroDireita2);
    cpShapeFree(ud->shape);
    cpBodyFree(zagueiroDireita2);

    ud = cpBodyGetUserData(zagueiroDireita3);
    cpShapeFree(ud->shape);
    cpBodyFree(zagueiroDireita3);

    ud = cpBodyGetUserData(atacanteDireita1);
    cpShapeFree(ud->shape);
    cpBodyFree(atacanteDireita1);

    ud = cpBodyGetUserData(atacanteDireita2);
    cpShapeFree(ud->shape);
    cpBodyFree(atacanteDireita2);

    //Limpando as paredes
    cpShapeFree(travessaoD1);
    cpShapeFree(travessaoD2);
    cpShapeFree(travessaoE1);
    cpShapeFree(travessaoD1);
    cpShapeFree(leftWall);
    cpShapeFree(rightWall);
    cpShapeFree(topWall);
    cpShapeFree(bottomWall);

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