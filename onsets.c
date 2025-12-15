#include "m_pd.h"
#include "math.h"

static t_class *onsets_class; /* variable permanente ou statique */

typedef struct _onsets {
  t_object  x_obj;
  t_float threshold;

} t_onsets;

void threshold_set(t_onsets *x, t_floatarg f)
{
  x->threshold = f;
}


void onsets_bang(t_onsets *x)
{
  (void)x;
  t_symbol *nameAudioArray = gensym("audio");
  t_symbol *rms = gensym("rms");
  t_symbol *onsets = gensym("onsets");

  t_garray *a;
  t_garray *r;
  t_garray *o;

  t_word *vec = NULL;
  t_word *vecRms = NULL;
  t_word *vecOnsets = NULL;
  
  int npointsAudio = 0;
  int npointsRms = 0;
  int npointsOnsets = 0;

  a = (t_garray *)pd_findbyclass(nameAudioArray, garray_class);
  r = (t_garray *)pd_findbyclass(rms, garray_class);
  o = (t_garray *)pd_findbyclass(onsets, garray_class);

  if (a == NULL || r == NULL || o == NULL)
      pd_error(x, "%s, %s, %s: no such array", nameAudioArray->s_name,rms->s_name, onsets->s_name);
  else {
      /* récupérer le buffer : garray_getfloatwords renvoie non‑zéro si OK */
      int ok = garray_getfloatwords(a, &npointsAudio, &vec);
      int ok2 = garray_getfloatwords(r, &npointsRms, &vecRms);
      int ok3 = garray_getfloatwords(o, &npointsOnsets, &vecOnsets);
      if (!ok || !ok2 || !ok3) {
          pd_error(x, "%s, %s, %s: bad template for onsets", nameAudioArray->s_name,rms->s_name, onsets->s_name);
      } else {

            t_float sr = sys_getsr();
            if (sr <= 0) sr = 44100.0f;            /* si pas d'audio */

            t_float threshold = x->threshold;

            const t_float window_ms = 10.0f;
            int window_samples = (int)roundf((window_ms / 1000.0f) * sr);
            if (window_samples <= 0) window_samples = 1;

            int nbwin = npointsAudio / window_samples; /* nombre de fenetres, pas d'overlap*/

            
            const t_float dist_min_ms = 50.0f; /*distance minimale entre les onsets*/
            int dist_min_samples = (int)roundf((dist_min_ms / 1000.0f) * sr); /* en nb d'echantillons*/

            for (int i = 0; i < npointsRms; i++) vecRms[i].w_float = 0.0f;
            for (int i = 0; i < npointsOnsets; i++) vecOnsets[i].w_float = 0.0f; /*on initialise les array a 0*/

            if (nbwin <= 0) {
              pd_error(x, "no complete windows");
            } else {
              if (nbwin > npointsRms) {
                pd_error(x, "rms array too small");
                nbwin = npointsRms;
              }

               /* calcul des RMS par fenêtre */
               for (int i = 0; i < nbwin; ++i) {
                  t_float sum = 0.0f;
                   int start = i * window_samples;
                   int end = start + window_samples;
                   if (end > npointsAudio) end = npointsAudio;
                   int len = end - start;
                   if (len <= 0) { vecRms[i].w_float = 0.0f; continue; }
                   for (int j = start; j < end; ++j) {
                      t_float s = vec[j].w_float;
                       sum += s * s;
                   }
                  vecRms[i].w_float = sqrtf(sum / (t_float)len);
                }

                 
               int last_onset_sample = -dist_min_samples;
              int onset_count = 0; /* indice dans l'array onsets  */
                for (int i = 1; i < nbwin; ++i) {
                    t_float delta = vecRms[i].w_float - vecRms[i-1].w_float;
                    int onset_sample = i * window_samples;
                    if (delta > threshold && (onset_sample - last_onset_sample) >= dist_min_samples) {

                        if (onset_count < npointsOnsets) {
                            /* stocker le numéro d'échantillon */
                            vecOnsets[onset_count].w_float = (t_float)onset_sample;
                            /* si vous préférez stocker en ms : */
                              /* vecOnsets[onset_count].w_float = ((t_float)onset_sample / sr) * 1000.0f; */
                            onset_count++;
                            last_onset_sample = onset_sample;
                        } else {
                            pd_error(x, "onsets: onsets array trop petit (besoin de %d entrées, taille %d)", onset_count+1, npointsOnsets);
                           break;
                        }
                    }
                }

                garray_redraw(r);
                garray_redraw(o);



            }
      }
  }
}

void *onsets_new(t_floatarg f)
{
  t_onsets *x = (t_onsets *)pd_new(onsets_class);
  x->threshold = f;
  return (void *)x;
}

void onsets_setup(void) {
  onsets_class = class_new(gensym("onsets"),
        (t_newmethod)onsets_new,
        0, sizeof(t_onsets),
        CLASS_DEFAULT, A_DEFFLOAT, 0);
  class_addbang(onsets_class, onsets_bang);

  class_addmethod(onsets_class, (t_method)threshold_set,
      gensym("threshold"), A_FLOAT, 0);
  
}