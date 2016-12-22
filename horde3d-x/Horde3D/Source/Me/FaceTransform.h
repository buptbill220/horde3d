//
//  FaceTransform.hpp
//  Visionin
//
//  Created by fangming.fm on 16/8/29.
//  Copyright © 2016年 Visionin. All rights reserved.
//

#ifndef FaceTransform_hpp
#define FaceTransform_hpp

#include <stdio.h>
#include <vector>

#endif /* FaceTransform_hpp */
///////////////////////////////////////////////////////////
///	n: 要求的人脸关键点个数，2/3/4/16/32/75
///	返回: 人脸关键点【xy,xy,...】，nil代表人脸丢失
/*!	return key marks[][x,y]:
 [ 0,1 ]: eyeballs
 [  2  ]: mouth
 --------
 [  3  ]: nosetip
 --------
 [ 4,5 ]: inner eye corners
 [ 6,7 ]: outer eye corners
 [ 8,9 ]: eye line high points
 [10,11]: eyebow high points
 [ 12  ]: upper-lip
 [ 13  ]: lower-lip
 [14,15]: lip corners
 --------
 [16,17]: eye line low points
 [18,19]: inner eyebows
 [20,21]: outer eyebows
 [22,23]: lip line low points from center
 [24,25]: nose side points
 [ 26  ]: nose base
 [ 27  ]: jawtip
 [28,29]: cheek points
 [30,31]: temple points
 --------
 [32,33][34,35]: temple to cheek line points
 [36,37][38,39][40,41][42,43]: cheek to jaw line points
 [44,45]: eye line high points near inner corners
 [46,47]: eye line high points near outer corners
 [48,49]: eye line low points near inner corners
 [50,51]: eye line low points near outer corners
 [52,53]: eyebow points near inner corners
 [54,55]: eyebow points near outer corners
 [56,57][58,59]: lip line high points from corners
 [60,61]: lip line low points from corners
 [ 62  ]: upper-gum
 [ 63  ]: lower-gum
 [64,65]: gum corners
 [66,67]: gum high points from corners
 [68,69]: gum low points from corners
 [70,71]: nose bridge from root to tip
 [72,73]: nostrils
 [ 74  ]: nose root
 !*/

const int face_point = 75;

/*
 * 面部四个部分变换时方向
*/
#define FACE_NONE   0x00
#define FACE_NEAR   0x01
#define FACE_FAR    0x02
#define FACE_LMOVE  0x04
#define FACE_RMOVE  0x08
#define FACE_UMOVE  0x10
#define FACE_DMOVE  0x20

// 根据2点方向判断人脸方向
#define is_face_nod(n0, n1) (((n0 | n1) & (FACE_NEAR | FACE_FAR)) == (FACE_NEAR | FACE_FAR))
#define is_face_swing(n0, n1) (is_face_nod(n0, n1))
#define is_face_move(n0, n1) (((n0 & n1) & (FACE_LMOVE | FACE_RMOVE | FACE_UMOVE | FACE_DMOVE)) != FACE_NONE)
#define is_face_rotate(n0, n1) (((n0 | n1) & (FACE_LMOVE | FACE_RMOVE)) == (FACE_LMOVE | FACE_RMOVE) || ((n0 | n1) & (FACE_DMOVE | FACE_UMOVE)) == (FACE_DMOVE | FACE_UMOVE))

struct vs3d_face_s {
    int *position;  // 人脸坐标位置信息
    int dim;        // 每个位置的维数，默认3维（xy/xyz）
    int posnum;     // 人脸点个数
    unsigned int realmem;   // 实际内存大小
    vs3d_face_s(): position(NULL), dim(3), posnum(face_point), realmem(225){}
};

struct vs3d_face_site_s {
    std::vector<int> check_pos; // 检测部位位置0~74中之一，取多个点用平均值，鲁棒性强
    int site;                   // 检测出的方向
    int delta[3];               // 各个方向变换量
    vs3d_face_site_s(): site(FACE_NONE){ memset(delta, 0, sizeof(int) * 3); }
};

typedef vs3d_face_s vs3d_face_t;
typedef vs3d_face_site_s vs3d_face_site_t;

class FaceTransform {
public:
    explicit FaceTransform();
    ~FaceTransform();
    float* updateFace(int* position, int posnum = 75, int dim = 3);
protected:
    void calcTransform();
    void init();
private:
    // 人脸位置，保存上一次坐标
    vs3d_face_t *m_face;
    // 道具变换数据
    float *m_transform;
    // 方向分别是上额，右脸，下巴，左脸
    vs3d_face_site_t m_face_site[4];
};
