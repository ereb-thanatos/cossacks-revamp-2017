#define MaskLx 64
extern byte ResultMask[MaskLx*256];
// 1(x0,y0)
//  |\
//  |  \ s1
//s3|    \ 2
//  |    /
//  |  / s2
//  |/
// 3
//
// bm1,bm2,bm3-numbers of bitmaps in BitmapArray
// if bm1<bm2 then bm2 is over bm1
// s1 (0..2) - section of 1-2 
// s2 (0..2) - section of 2-3 
// s3 (0..2) - section of 3-1 
void PrepareIntersection1(int bm1,int bm2,int bm3,
						  int x0,int y0,
						  int s1,int s2,int s3,
						  RLCTable Masks,byte* BitmapsArray);
//
//                1
//         s1   /|
//            /  |
//        2 /    | s3
// (x01,y0) \    |
//            \  |
//         s2   \|
//                3
void PrepareIntersection2(int bm1,int bm2,int bm3,
						  int x0,int y01,
						  int s1,int s2,int s3,
						  RLCTable Masks,byte* BitmapsArray);
void ShowIntersectionBuffer();
int GetBmOfst(int i);