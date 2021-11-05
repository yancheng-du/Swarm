#ifndef director_hpp
#define director_hpp

bool director_initialize();
void director_dispose();

bool director_is_running();
void director_do_frame();
void director_process_events();

#endif /* director_hpp */
