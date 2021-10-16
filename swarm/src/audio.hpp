#ifndef audio_hpp
#define audio_hpp

#include "swarm.hpp"

bool audio_initialize();
void audio_dispose();

void audio_render(const swarm_t *swarm);

#endif /* audio_hpp */
