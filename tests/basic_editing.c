
#include "app.h"
#include <assert.h>
App g;
static void mask(int w,int h){sel_reset();g.doc.w=w;g.doc.h=h;sel_ensure();g.have_sel=1;}
int main(void){
Layer L={0};assert(layer_create(&L,2,3,L"test"));for(int i=0;i<6;i++){L.bgra[i*4]=i+1;L.bgra[i*4+3]=255;}
adj_rotate_quarters(&L,3);assert(L.w==3 && L.h==2 && L.bgra[0]==2 && L.bgra[8]==6 && L.bgra[12]==1);
adj_rotate_quarters(&L,1);for(int i=0;i<6;i++)assert(L.bgra[i*4]==i+1);
adj_rotate_quarters(&L,2);for(int i=0;i<6;i++)assert(L.bgra[i*4]==6-i);layer_free(&L);
assert(layer_create(&L,2,1,L"tone"));for(int k=0;k<3;k++){L.bgra[k]=50;L.bgra[4+k]=150;}L.bgra[3]=77;L.bgra[7]=255;
adj_auto_contrast(&L);assert(L.bgra[0]==0 && L.bgra[4]==255 && L.bgra[3]==77);
for(int k=0;k<3;k++){L.bgra[k]=90;L.bgra[4+k]=160;}adj_threshold(&L,128);assert(L.bgra[0]==0 && L.bgra[4]==255);
mask(2,1);g.sel[0]=128;g.sel[1]=0;for(int k=0;k<3;k++){L.bgra[k]=100;L.bgra[4+k]=100;}adj_threshold(&L,128);assert(L.bgra[0]==50 && L.bgra[4]==100);sel_reset();
for(int k=0;k<3;k++){L.bgra[k]=90;L.bgra[4+k]=160;}adj_posterize(&L,2);assert(L.bgra[0]==0 && L.bgra[4]==255);layer_free(&L);
mask(5,5);g.sel[12]=255;sel_modify(0,1);int count=0;for(int i=0;i<25;i++)count+=g.sel[i]>0;assert(count==9);
sel_modify(1,1);count=0;for(int i=0;i<25;i++)count+=g.sel[i]>0;assert(count==1 && g.sel[12]==255);
sel_modify(2,1);assert(g.sel[12]==28 && g.sel[6]==28 && g.sel[0]==0);sel_reset();
Document d={0},loaded={0};assert(doc_new(&d,2,2,0));for(int mode=0;mode<=BLEND_DIFFERENCE;mode++){d.layers[0].blend=mode;assert(io_save_dagrim(L"build/test-roundtrip.dagrim",&d));assert(io_load_dagrim(L"build/test-roundtrip.dagrim",&loaded));assert(loaded.layers[0].blend==mode);doc_free(&loaded);}doc_free(&d);
puts("PASS: rotations, tones, alpha, soft masks, selection modification, 8 blend mode round-trips");return 0;}
void doc_to_view(float x,float y,int *vx,int *vy){*vx=(int)x;*vy=(int)y;}
