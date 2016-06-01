#include "eye_models.h"
#include "stonyman.h"
#include "diskio.h"
#include "stm32l152d_eval_sdio_sd.h"
#include "math.h"

extern unsigned short val[112*112];
int8_t pred[2];
float pred_radius;
float last_r = 0;
uint8_t cider_colrow[2];

unsigned short num_subsample = 0;
unsigned short num_hidden = 0;
unsigned int bh_offset = 0;
unsigned int bo_offset = 0;
unsigned int mask_offset = 0;
unsigned int who_offset = 0;
unsigned int wih_offset = 0;
unsigned int fpn_offset = 0;
unsigned int col_fpn_offset = 0;

extern uint16_t model_data[];

// NOTE: Because C inlining is weird, these functions are defined in eye_models.h
extern inline void init_streamstats(StreamStats *ss);
extern inline void update_streamstats(StreamStats *ss, uint16_t *pixels, uint16_t this_pixel);
// extern inline void update_streamstats(StreamStats *ss, uint16_t *pixels, uint16_t this_pixel, 
//                         int i_major, int j_minor, uint16_t mask_major, uint16_t mask_minor);

// Static keyword is not enforced in IAR. Please don't abuse this.
static void ann_predict_meanstd(uint16_t *pixels, float mean, float std);

static float tanh_values[] = {
-0.999909204263 ,
-0.999889102951 ,
-0.999864551701 ,
-0.999834565554 ,
-0.999797941612 ,
-0.999753210848 ,
-0.999698579284 ,
-0.99963185619 ,
-0.99955036646 ,
-0.999450843688 ,
-0.999329299739 ,
-0.99918086567 ,
-0.998999597786 ,
-0.998778241281 ,
-0.998507942332 ,
-0.998177897611 ,
-0.997774927934 ,
-0.997282960099 ,
-0.99668239784 ,
-0.995949359222 ,
-0.995054753687 ,
-0.993963167351 ,
-0.992631520201 ,
-0.991007453678 ,
-0.989027402201 ,
-0.986614298151 ,
-0.983674857694 ,
-0.980096396266 ,
-0.975743130031 ,
-0.970451936613 ,
-0.964027580076 ,
-0.956237458128 ,
-0.946806012846 ,
-0.935409070603 ,
-0.921668554406 ,
-0.905148253645 ,
-0.885351648202 ,
-0.861723159313 ,
-0.833654607012 ,
-0.800499021761 ,
-0.761594155956 ,
-0.716297870199 ,
-0.664036770268 ,
-0.604367777117 ,
-0.537049566998 ,
-0.46211715726 ,
-0.379948962255 ,
-0.291312612452 ,
-0.197375320225 ,
-0.099667994625 ,
0.0 ,
0.099667994625 ,
0.197375320225 ,
0.291312612452 ,
0.379948962255 ,
0.46211715726 ,
0.537049566998 ,
0.604367777117 ,
0.664036770268 ,
0.716297870199 ,
0.761594155956 ,
0.800499021761 ,
0.833654607012 ,
0.861723159313 ,
0.885351648202 ,
0.905148253645 ,
0.921668554406 ,
0.935409070603 ,
0.946806012846 ,
0.956237458128 ,
0.964027580076 ,
0.970451936613 ,
0.975743130031 ,
0.980096396266 ,
0.983674857694 ,
0.986614298151 ,
0.989027402201 ,
0.991007453678 ,
0.992631520201 ,
0.993963167351 ,
0.995054753687 ,
0.995949359222 ,
0.99668239784 ,
0.997282960099 ,
0.997774927934 ,
0.998177897611 ,
0.998507942332 ,
0.998778241281 ,
0.998999597786 ,
0.99918086567 ,
0.999329299739 ,
0.999450843688 ,
0.99955036646 ,
0.99963185619 ,
0.999698579284 ,
0.999753210848 ,
0.999797941612 ,
0.999834565554 ,
0.999864551701 ,
0.999889102951 ,
0.999909204263 ,
};

// Commenting this out until we need it for something
//static float calc_std(uint16_t img[])
//{
//    float M = 0.0;
//    float S = 0.0;
//    int k = 1;
//    for (int i = 0; i < (112 * 112); i++) {
//      float value = (float)img[i];
//      float tmpM = M;
//      M += (value - tmpM) / k;
//      S += (value - tmpM) * (value - M);
//      k++;
//    }
//    float temp_res = sqrt(S / (k-1));
//    return sqrt(S / (k-1));
//}

static float tanh_approx(float input)
{
    if (input <= -5)
        return -1;
    else if (input >= 5)
        return 1;

    // Offset input value
    input += 5;

    // Each index corresponds to 0.1 in the range from -5 to 5 (0 to 10 after offset)
    int lower_index = (int)(input * 10);
    int upper_index = lower_index + 1;

    // Just in case precision or rounding issues push us into the clipped range
    if (lower_index < 0)
        return -1;
    if (lower_index >= 99)
        return 1;

    float x0 = (float)(lower_index) / 10, x1 = (float)(upper_index) / 10;
    float y0 = tanh_values[lower_index], y1 = tanh_values[upper_index];

    float approx = y0 + (y1 - y0) * ((input - x0) / (x1 - x0));

    return approx;
}

#ifdef USE_PARAM_FILE
void read_cider_params()
{
    num_subsample = model_data[PARAMS_START];
    num_hidden = model_data[PARAMS_START + 1];

    if (num_subsample == 0 || num_hidden == 0)      return;

    bh_offset = PARAMS_START + 2;
    bo_offset = bh_offset + num_hidden * 2;
    mask_offset = bo_offset + 4;
    who_offset = mask_offset + num_subsample * 2;
    wih_offset = who_offset + num_hidden * 2 * 2;
}
#endif // USE_PARAM_FILE

void ann_predict(uint16_t *pixels)
{
        // Mean computation
    uint32_t pixel_sum = 0;
    for (int i = 0; i < num_subsample; i++)
        pixel_sum += pixels[i];
    float mean = (float)pixel_sum / num_subsample;

    // Standard deviation computation
    float diff_sum = 0;
    for (int i = 0; i < num_subsample; i++)
        diff_sum += (pixels[i] - mean) * (pixels[i] - mean);
    float std = sqrt(diff_sum / num_subsample);

    ann_predict_meanstd(pixels, mean, std);
}

void ann_predict2(uint16_t* pixels, StreamStats *ss)
{
    float mean = (float)ss->pixel_sum / num_subsample;
    float std = sqrt(ss->S / (ss->k-1));

    ann_predict_meanstd(pixels, mean, std);
}

static void ann_predict_meanstd(uint16_t *pixels, float mean, float std)
{
    int i, j;
    float ah[6];
    float x, x_val, y_val;

    for (i = 0; i < num_hidden; i++)  {
        ah[i] = BH(i);
    }

    uint16_t subsample;
    for (i = 0; i < num_subsample; i++) {
        subsample = pixels[i];
        x = (float)(subsample - mean) / std;

        for (j = 0; j < num_hidden; j++) {
            float wih_tmp = WIH(i, j);
            ah[j] += x * wih_tmp;
        }
    }

    x_val = BO(0);
    y_val = BO(1);

    for (i = 0; i < num_hidden; i++) {
        x_val += WHO(i, 0) * tanh_approx(ah[i]);
        y_val += WHO(i, 1) * tanh_approx(ah[i]);
    }

    // pred = global for storing prediction values
    pred[PRED_X] = (unsigned short)((x_val * 112) + 0.5);
    pred[PRED_Y] = (unsigned short)((y_val * 111) + 0.5);

    return;
}

bool run_cider()
{
  uint8_t row_edges[6] = {0, 0, 0, 0, 0, 0}, col_edges[6] = {0, 0, 0, 0, 0, 0};
  bool pupil_found = false;
  
  // Make sure the selected row / col is in [0, 111]
  cider_colrow[1] = (uint8_t)((pred[1] < 0 ? 0 : pred[1]) > 111 ? 111 : pred[1]);
  cider_colrow[0] = (uint8_t)((pred[0] < 0 ? 0 : pred[0]) > 111 ? 111 : pred[0]);
  
  uint8_t col_start = cider_colrow[1], row_start = cider_colrow[0];
  
  float best_ratio = 0, best_r = 0;
  uint8_t best_center[2] = {0, 0};

  // Configure ADC to read only from eye camera
  config_adc_select(EYE_CAM);
  
  // Read the selected row and column from the camera
  uint16_t row[112], col[112];
  stony_cider_line(col_start, row, SEL_ROW);
  stony_cider_line(row_start, col, SEL_COL);

  config_adc_default();
  
  // Find the edges of the pupil in the selected row and column
  find_pupil_edge(row_start, row_edges, row);
  find_pupil_edge(col_start, col_edges, col);

  // Evaluate all candidate edges found during the search
  for (uint8_t i = 0; (row_edges[i] != 0 || i==0) && i < 6; i += 2) {
    // Pupil can't be smaller than 4 pixels across
    if ((row_edges[i] - row_edges[i + 1]) < 4 && (row_edges[i] - row_edges[i + 1]) > -4)
      continue;
    
    for (uint8_t j = 0; (col_edges[j] != 0 || j==0) && j < 6; j += 2) {
      // Pupil can't be smaller than 4 pixels across
      if ((col_edges[j] - col_edges[j + 1]) < 4 && (col_edges[j] - col_edges[j + 1]) > -4)
        continue;
      
      // Calculate the midpoint between the two edges
      float x_mid, y_mid;
      x_mid = (row_edges[i] + row_edges[i + 1]) / (float)2;
      y_mid = (col_edges[j] + col_edges[j + 1]) / (float)2;
      
      // Calculate the two radius values (for redundancy checking)
      float r1, r2;
      r1 = sqrt(((x_mid - row_edges[i]) * (x_mid - row_edges[i])) + ((y_mid - col_start) * (y_mid - col_start)));
      r2 = sqrt(((x_mid - row_start) * (x_mid - row_start)) + ((y_mid - col_edges[j]) * (y_mid - col_edges[j])));
      
      // Test that the two radii are relatively close to each other
      // (if not, we probably missed the pupil)
      float ratio = r1 / r2;
      if ((ratio < 0.6) || (ratio > (1/0.6)) || fabs(ratio - 1) > fabs(best_ratio - 1))
          continue;
      
      // Average the two results and compare with previous frame
      // (if there's a huge change, something probably went wrong)
      float r = (r1 + r2) / 2;
      if (last_r != 0 && (r / last_r < 0.75 || r / last_r > 1/0.75))
          continue;
      
      // Check / update the best result so far
      pupil_found = true;
      best_ratio = ratio; best_r = r;
      best_center[0] = x_mid >= 0 ? (uint8_t)(x_mid+0.5) : (uint8_t)(x_mid-0.5);
      best_center[1] = y_mid >= 0 ? (uint8_t)(y_mid+0.5) : (uint8_t)(y_mid-0.5);
    }
  }
  
  pred[0] = best_center[0];
  pred[1] = best_center[1];
  pred_radius = best_r;
  last_r = best_r;
  
  // Mark the next data packet with whether we found the pupil or not
  mark_cider_packet(!pupil_found);

  return pupil_found;
}


void find_pupil_edge(uint8_t start_point, uint8_t* edges, uint16_t* pixels)
{
  uint16_t med_buf[2], next_pixel;
  uint8_t med_idx, small_val, reg_size, edge_idx;
  uint8_t peak_after, local_regions[3], lr_idx, lr_min;
  uint8_t peaks[53], peak_idx, spec_regions[53], spec_idx;
  int16_t conv_sum, conv_abs, reg_sum, edge_mean, region_means[53];
  int16_t edge_detect[106];
  
  uint8_t in_specular = 0, new_peak = 0; 
  
  // First do median filtering
  med_buf[0] = pixels[0];
  med_buf[1] = pixels[1];
  med_idx = 0;
  small_val = (pixels[0] < pixels[1]) ? 0 : 1;
  
  for (uint8_t i = 2; i < 112; i++) {
    next_pixel = pixels[i];
    
    if (next_pixel < med_buf[small_val]) {
      pixels[i - 1] = med_buf[small_val];
      small_val = med_idx;
    } else if (next_pixel > med_buf[!small_val]) {
      pixels[i - 1] = med_buf[!small_val];
      small_val = !med_idx;
    } else {
      pixels[i - 1] = next_pixel;
    }
    
    med_buf[med_idx] = next_pixel;
    med_idx = !med_idx;
  }
  
  // Then percentile clamping
  uint16_t perc_val = quick_percentile(pixels);
  for (uint8_t i = 0; i < 112; i++) {
    if (pixels[i] < perc_val)   pixels[i] = perc_val;
  }
  
  // Next, do convolution
  conv_sum = -pixels[0] - pixels[1] - pixels[2] + pixels[4] + pixels[5] + pixels[6];
  conv_abs = (conv_sum < 0) ? (-conv_sum) : (conv_sum);
  edge_detect[0] = conv_abs;
  edge_mean = 0;
  for (uint8_t i = 4; i < 108; i++) {
    conv_sum += pixels[i - 4];
    conv_sum -= pixels[i - 1];
    conv_sum -= pixels[i];
    conv_sum += pixels[i + 3];
    
    conv_abs = (conv_sum < 0) ? (-conv_sum) : (conv_sum);
    
    edge_detect[i - 3] = conv_abs;
    edge_mean += conv_abs;
  }
  edge_mean /= 106;
  
  // Then peak identification (+ weeding out peaks resulting from specular reflection)
  // and calculating region means (+ identifying specular regions)
  peaks[0] = 0; reg_sum = pixels[1] + pixels[2] + pixels[3];
  peak_idx = 1; in_specular = 0; new_peak = 0; peak_after = 0;
  for (uint8_t i = 1; i < 105; i++) {
    if (edge_detect[i] > SPEC_THRESH) {
      if (in_specular == 0) {
        in_specular = 1;
        if (i > 1 && peaks[peak_idx - 1] != (i - 1)) {
          peaks[peak_idx] = i - 1;
          spec_regions[spec_idx] = peak_idx;
          peak_idx++; new_peak = 1;
        } else {
          spec_regions[spec_idx] = peak_idx - 1;
        }
        spec_idx++;
      } else if (in_specular == 2) {
        in_specular++;
      }
    } else if (edge_detect[i] < SPEC_THRESH && in_specular != 0) {
      if (in_specular == 1)
        in_specular++;
      else if (in_specular == 3) {
        in_specular = 0;
        peaks[peak_idx] = i;
        peak_idx++; new_peak = 1;
      }
    } else {
      if (edge_detect[i] >= edge_detect[i - 1] && edge_detect[i] > edge_detect[i + 1] && edge_detect[i] > edge_mean) {
        peaks[peak_idx] = i;
        peak_idx++; new_peak = 1;
      }
    }
    
    reg_sum += pixels[i + 3];
    
    if (new_peak == 1) {
      reg_size = peaks[peak_idx - 1] - peaks[peak_idx - 2] + (peak_idx == 2 ? 3 : 0);
      
      // If we retroactively made the previous point a peak, need to adjust the mean calculation
      if (in_specular == 1) {
        region_means[peak_idx - 2] = (reg_sum - pixels[i + 3]) / reg_size;
        reg_sum = pixels[i + 3];
      } else {
        region_means[peak_idx - 2] = reg_sum / reg_size;
        reg_sum = 0;
      }
      
      new_peak = 0;
      if (peak_after == 0 && peaks[peak_idx - 1] > start_point)
        peak_after = peak_idx - 1;
    }
  }
  
  // Set last peak as last pixel
  peaks[peak_idx] = 111;
  peak_idx++;
  reg_sum += pixels[108] + pixels[109] + pixels[110] + pixels[111];
  reg_size = peaks[peak_idx - 1] - peaks[peak_idx - 2];
  region_means[peak_idx - 2] = reg_sum / reg_size;
  
  if (peak_after == 0)
    peak_after = peak_idx - 1;
  
  // Identify the local regions around the start point
  lr_idx = 0;
  if (peak_after > 2) {
    local_regions[lr_idx] = peak_after - 2;
    lr_idx++;
  }
  
  local_regions[lr_idx] = peak_after - 1;
  lr_idx++;
  
  if (peak_after < peak_idx - 1) {
    local_regions[lr_idx] = peak_after;
    lr_idx++;
  }
  
  // Select the local region with the lowest mean
  lr_min = 0;
  for (uint8_t i = 1; i < lr_idx; i++) {
    if (region_means[local_regions[i]] < region_means[local_regions[lr_min]])
      lr_min = i;
  }
  
  edges[0] = peaks[local_regions[lr_min]];
  edges[0] += (edges[0] == 0 ? 0 : CONV_OFFSET);
  edges[1] = peaks[local_regions[lr_min]+1];
  edges[1] += (edges[1] == 111 ? 0 : CONV_OFFSET);
  edge_idx = 2;
  
  // Commented out b/c I need to debug this... (AMM)
//  // Check if the region has a specular point on either end
//  for (uint8_t i = 0; i < spec_idx; i++) {
//    if (spec_regions[i] == local_regions[lr_min] - 1) {
//      edges[edge_idx] = peaks[local_regions[lr_min] - 1] + CONV_OFFSET;
//      edges[edge_idx+1] = peaks[local_regions[lr_min] + 1] + CONV_OFFSET;
//      edge_idx += 2;
//    } else if (spec_regions[i] == local_regions[lr_min] + 1) {
//      edges[edge_idx] = peaks[local_regions[lr_min]] + CONV_OFFSET;
//      edges[edge_idx+1] = peaks[local_regions[lr_min] + 2] + CONV_OFFSET;
//      edge_idx += 2;
//    }
//  }
  
  return;       // Edge data is stored in argument array
}

uint16_t quick_percentile(uint16_t *base_row)
{
  uint16_t row[112];
  uint16_t r, w, mid;
  
  for (uint16_t i = 0; i < 112; i++) {
    row[i] = base_row[i];
  }
  
  uint8_t from = 0, to = 111;
  
  // Percentile value is hardcoded so it doesn't have to be computed at runtime
  uint8_t k = (((112 * 100) * CIDER_PERCENTILE) / 10000);
  
  // if from == to we reached the kth element
  while (from < to) {
    r = from, w = to;
    mid = row[(r + w) / 2];
    
    // stop if the reader and writer meets
    while (r < w) {
      
      if (row[r] >= mid) { // put the large values at the end
        uint16_t tmp = row[w];
        row[w] = row[r];
        row[r] = tmp;
        w--;
      } else { // the value is smaller than the pivot, skip
        r++;
      }
    }
    
    // if we stepped up (r++) we need to step one down
    if (row[r] > mid && r != 0)    
      r--;
    
    // the r pointer is on the end of the first k elements
    if (k <= r) {
      to = r;
    } else {
      from = r + 1;
    }
  }
  
  return row[k];
}