#pragma once

namespace lgr {

class state;
class input;

void initialize_meshless_V(state& s);
void initialize_meshless_grad_val_N(state& s);
void update_meshless_h_min_inball(input const&, state& s);
void update_meshless_h_art(state& s);

}
