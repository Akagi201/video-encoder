/*
 * @file x264_encoder.c
 * @author Akagi201
 * @date 2014/12/31
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "lwlog/lwlog.h"

#include "x264.h"

int main(int argc, char **argv) {

    int ret = 0;
    int y_size = 0;
    int i = 0;
    int j = 0;

    //Encode 50 frame
    //if set 0, encode all frame
    int frame_num = 50;
    int csp = X264_CSP_I420;
    int width = 640;
    int height = 360;

    int i_nal = 0;
    x264_nal_t *p_nals = NULL;
    x264_t *p_handle = NULL;
    x264_picture_t *p_pic_in = (x264_picture_t *) malloc(sizeof(x264_picture_t));
    x264_picture_t *p_pic_out = (x264_picture_t *) malloc(sizeof(x264_picture_t));
    x264_param_t *p_param = (x264_param_t *) malloc(sizeof(x264_param_t));

    //FILE* fp_src  = fopen("../cuc_ieschool_640x360_yuv444p.yuv", "rb");
    FILE *fp_src = fopen("../cuc_ieschool_640x360_yuv420p.yuv", "rb");

    FILE *fp_dst = fopen("cuc_ieschool.h264", "wb");

    //Check
    if ((NULL == fp_src) || (NULL == fp_dst)) {
        lwlog_err("Open files error.");
        return -1;
    }

    x264_param_default(p_param);
    p_param->i_width = width;
    p_param->i_height = height;
    /*
    //Param
    p_param->i_log_level  = X264_LOG_DEBUG;
    p_param->i_threads  = X264_SYNC_LOOKAHEAD_AUTO;
    p_param->i_frame_total = 0;
    p_param->i_keyint_max = 10;
    p_param->i_bframe  = 5;
    p_param->b_open_gop  = 0;
    p_param->i_bframe_pyramid = 0;
    p_param->rc.i_qp_constant=0;
    p_param->rc.i_qp_max=0;
    p_param->rc.i_qp_min=0;
    p_param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
    p_param->i_fps_den  = 1;
    p_param->i_fps_num  = 25;
    p_param->i_timebase_den = p_param->i_fps_num;
    p_param->i_timebase_num = p_param->i_fps_den;
    */
    p_param->i_csp = csp;
    x264_param_apply_profile(p_param, x264_profile_names[5]);

    p_handle = x264_encoder_open(p_param);

    x264_picture_init(p_pic_out);
    x264_picture_alloc(p_pic_in, csp, p_param->i_width, p_param->i_height);

    //ret = x264_encoder_headers(p_handle, &p_nals, &i_nal);

    y_size = p_param->i_width * p_param->i_height;
    //detect frame number
    if (0 == frame_num) {
        fseek(fp_src, 0, SEEK_END);
        switch (csp) {
            case X264_CSP_I444:
                frame_num = ftell(fp_src) / (y_size * 3);
                break;
            case X264_CSP_I420:
                frame_num = ftell(fp_src) / (y_size * 3 / 2);
                break;
            default:
                lwlog_err("Colorspace Not Support");
                return -1;
        }
        fseek(fp_src, 0, SEEK_SET);
    }

    //Loop to Encode
    for (i = 0; i < frame_num; ++i) {
        switch (csp) {
            case X264_CSP_I444: {
                fread(p_pic_in->img.plane[0], y_size, 1, fp_src);    //Y
                fread(p_pic_in->img.plane[1], y_size, 1, fp_src);    //U
                fread(p_pic_in->img.plane[2], y_size, 1, fp_src);    //V
                break;
            }
            case X264_CSP_I420: {
                fread(p_pic_in->img.plane[0], y_size, 1, fp_src);    //Y
                fread(p_pic_in->img.plane[1], y_size / 4, 1, fp_src);    //U
                fread(p_pic_in->img.plane[2], y_size / 4, 1, fp_src);    //V
                break;
            }
            default: {
                lwlog_err("Colorspace Not Support.\n");
                return -1;
            }
        }
        p_pic_in->i_pts = i;

        ret = x264_encoder_encode(p_handle, &p_nals, &i_nal, p_pic_in, p_pic_out);
        if (ret < 0) {
            lwlog_err("x264_encoder_encode error");
            return -1;
        }

        lwlog_info("Succeed encode frame: %5d\n", i);

        for (j = 0; j < i_nal; ++j) {
            fwrite(p_nals[j].p_payload, 1, p_nals[j].i_payload, fp_dst);
        }
    }
    i = 0;
    //flush encoder
    while (1) {
        ret = x264_encoder_encode(p_handle, &p_nals, &i_nal, NULL, p_pic_out);
        if (0 == ret) {
            break;
        }
        lwlog_info("Flush 1 frame.");
        for (j = 0; j < i_nal; ++j) {
            fwrite(p_nals[j].p_payload, 1, p_nals[j].i_payload, fp_dst);
        }
        ++i;
    }
    x264_picture_clean(p_pic_in);
    x264_encoder_close(p_handle);
    p_handle = NULL;

    free(p_pic_in);
    free(p_pic_out);
    free(p_param);

    fclose(fp_src);
    fclose(fp_dst);

    return 0;
}
