// arbiter.cpp
#include "arbiter.h"
#include "packet.h"

void arbiter ::func() {
  sc_uint<1> v_connected_input[5]; // set when input is connected to an output
  sc_uint<1> v_reserved_output[6]; // set when output is reserved by a input
                                   // (one output more for simple coding)
  sc_uint<3> v_req[5];
  sc_uint<5> v_free; // status of output in term of being free
  sc_uint<4> v_id;
  sc_uint<5> v_arbit;
  sc_uint<15> v_select;

  for (int i = 0; i < 5; i++) {
    v_connected_input[i] = 0;
    v_reserved_output[i] = 0;
    v_req[i] = 0;
  }
  v_reserved_output[5] = 0;

  v_free = 0; // '11111'
  v_arbit = 0;
  v_select = 0;

  // functionality
  while (true) {
    wait();
    // Recompute availability each cycle; stale free bits must not bypass backpressure.
    v_free = 0;
    grant0.write(0); // reset grant
    grant1.write(0); // reset grant
    grant2.write(0); // reset grant
    grant3.write(0); // reset grant
    grant4.write(0); // reset grant

    if (!free_out0.read()) {
      v_free = v_free | 1;
    } // set the bit 0 showing the output 0 is free
    if (!free_out1.read()) {
      v_free = v_free | 2;
    }
    if (!free_out2.read()) {
      v_free = v_free | 4;
    }
    if (!free_out3.read()) {
      v_free = v_free | 8;
    }
    if (!free_out4.read()) {
      v_free = v_free | 16;
    }

    v_id = arbiter_id.read();

    // Processing input 0
    if (!req0.read()[4]) // if FIFO buffer is not empty
    {
      // For header flit, determine routing
      if (!v_connected_input[0]) {
        sc_uint<4> dest = req0.read().range(3, 0); // dest[3:0] from req[3:0]
        sc_uint<2> x_dest = dest.range(1, 0);
        sc_uint<2> y_dest = dest.range(3, 2);
        sc_uint<2> x_current = v_id.range(1, 0);
        sc_uint<2> y_current = v_id.range(3, 2);

        if (x_current < x_dest)
          v_req[0] = 3; // East
        else if (x_current > x_dest)
          v_req[0] = 5; // West
        else if (y_current < y_dest)
          v_req[0] = 4; // South
        else if (y_current > y_dest)
          v_req[0] = 2; // North
        else
          v_req[0] = 1; // Local
      }

      // Select output based on routing for header or body flits
      switch (v_req[0]) {
      case 1:
        v_arbit = v_free & 1;
        break;
      case 2:
        v_arbit = v_free & 2;
        break;
      case 3:
        v_arbit = v_free & 4;
        break;
      case 4:
        v_arbit = v_free & 8;
        break;
      case 5:
        v_arbit = v_free & 16;
        break;
      default:
        break;
      }

      if (!v_connected_input[0] &&
          v_reserved_output[v_req[0]]) // if input is not connected
      {
        v_arbit = 0;
      }
      if (v_arbit != 0) {
        grant0.write(1); // set grant
        v_select.range(2, 0) = v_req[0];
        v_free = v_free & (~v_arbit);    // inactive the related output
        v_connected_input[0] = 1;        // input 0 is connected
        v_reserved_output[v_req[0]] = 1; // output is reserved
        if (req0.read()[5]) {
          v_connected_input[0] = 0;
          v_reserved_output[v_req[0]] = 0;
        } // if it is tail flit, reset connection and reservation
      }
    }

    // Processing input 1
    if (!req1.read()[4]) {
      if (!v_connected_input[1]) {
        sc_uint<4> dest = req1.read().range(3, 0);
        sc_uint<2> x_dest = dest.range(1, 0);
        sc_uint<2> y_dest = dest.range(3, 2);
        sc_uint<2> x_current = v_id.range(1, 0);
        sc_uint<2> y_current = v_id.range(3, 2);

        if (x_current < x_dest)
          v_req[1] = 3;
        else if (x_current > x_dest)
          v_req[1] = 5;
        else if (y_current < y_dest)
          v_req[1] = 4;
        else if (y_current > y_dest)
          v_req[1] = 2;
        else
          v_req[1] = 1;
      }

      switch (v_req[1]) {
      case 1:
        v_arbit = v_free & 1;
        break;
      case 2:
        v_arbit = v_free & 2;
        break;
      case 3:
        v_arbit = v_free & 4;
        break;
      case 4:
        v_arbit = v_free & 8;
        break;
      case 5:
        v_arbit = v_free & 16;
        break;
      default:
        break;
      }

      if (!v_connected_input[1] && v_reserved_output[v_req[1]])
        v_arbit = 0;
      if (v_arbit != 0) {
        grant1.write(1);
        v_select.range(5, 3) = v_req[1];
        v_free = v_free & (~v_arbit);
        v_connected_input[1] = 1;
        v_reserved_output[v_req[1]] = 1;
        if (req1.read()[5]) {
          v_connected_input[1] = 0;
          v_reserved_output[v_req[1]] = 0;
        }
      }
    }

    // Repeat similar logic for inputs 2, 3, and 4
    // Processing input 2
    if (!req2.read()[4]) {
      if (!v_connected_input[2]) {
        sc_uint<4> dest = req2.read().range(3, 0);
        sc_uint<2> x_dest = dest.range(1, 0);
        sc_uint<2> y_dest = dest.range(3, 2);
        sc_uint<2> x_current = v_id.range(1, 0);
        sc_uint<2> y_current = v_id.range(3, 2);

        if (x_current < x_dest)
          v_req[2] = 3;
        else if (x_current > x_dest)
          v_req[2] = 5;
        else if (y_current < y_dest)
          v_req[2] = 4;
        else if (y_current > y_dest)
          v_req[2] = 2;
        else
          v_req[2] = 1;
      }

      switch (v_req[2]) {
      case 1:
        v_arbit = v_free & 1;
        break;
      case 2:
        v_arbit = v_free & 2;
        break;
      case 3:
        v_arbit = v_free & 4;
        break;
      case 4:
        v_arbit = v_free & 8;
        break;
      case 5:
        v_arbit = v_free & 16;
        break;
      default:
        break;
      }

      if (!v_connected_input[2] && v_reserved_output[v_req[2]])
        v_arbit = 0;
      if (v_arbit != 0) {
        grant2.write(1);
        v_select.range(8, 6) = v_req[2];
        v_free = v_free & (~v_arbit);
        v_connected_input[2] = 1;
        v_reserved_output[v_req[2]] = 1;
        if (req2.read()[5]) {
          v_connected_input[2] = 0;
          v_reserved_output[v_req[2]] = 0;
        }
      }
    }

    // Processing input 3
    if (!req3.read()[4]) {
      if (!v_connected_input[3]) {
        sc_uint<4> dest = req3.read().range(3, 0);
        sc_uint<2> x_dest = dest.range(1, 0);
        sc_uint<2> y_dest = dest.range(3, 2);
        sc_uint<2> x_current = v_id.range(1, 0);
        sc_uint<2> y_current = v_id.range(3, 2);

        if (x_current < x_dest)
          v_req[3] = 3;
        else if (x_current > x_dest)
          v_req[3] = 5;
        else if (y_current < y_dest)
          v_req[3] = 4;
        else if (y_current > y_dest)
          v_req[3] = 2;
        else
          v_req[3] = 1;
      }

      switch (v_req[3]) {
      case 1:
        v_arbit = v_free & 1;
        break;
      case 2:
        v_arbit = v_free & 2;
        break;
      case 3:
        v_arbit = v_free & 4;
        break;
      case 4:
        v_arbit = v_free & 8;
        break;
      case 5:
        v_arbit = v_free & 16;
        break;
      default:
        break;
      }

      if (!v_connected_input[3] && v_reserved_output[v_req[3]])
        v_arbit = 0;
      if (v_arbit != 0) {
        grant3.write(1);
        v_select.range(11, 9) = v_req[3];
        v_free = v_free & (~v_arbit);
        v_connected_input[3] = 1;
        v_reserved_output[v_req[3]] = 1;
        if (req3.read()[5]) {
          v_connected_input[3] = 0;
          v_reserved_output[v_req[3]] = 0;
        }
      }
    }

    // Processing input 4
    if (!req4.read()[4]) {
      if (!v_connected_input[4]) {
        sc_uint<4> dest = req4.read().range(3, 0);
        sc_uint<2> x_dest = dest.range(1, 0);
        sc_uint<2> y_dest = dest.range(3, 2);
        sc_uint<2> x_current = v_id.range(1, 0);
        sc_uint<2> y_current = v_id.range(3, 2);

        if (x_current < x_dest)
          v_req[4] = 3;
        else if (x_current > x_dest)
          v_req[4] = 5;
        else if (y_current < y_dest)
          v_req[4] = 4;
        else if (y_current > y_dest)
          v_req[4] = 2;
        else
          v_req[4] = 1;
      }

      switch (v_req[4]) {
      case 1:
        v_arbit = v_free & 1;
        break;
      case 2:
        v_arbit = v_free & 2;
        break;
      case 3:
        v_arbit = v_free & 4;
        break;
      case 4:
        v_arbit = v_free & 8;
        break;
      case 5:
        v_arbit = v_free & 16;
        break;
      default:
        break;
      }

      if (!v_connected_input[4] && v_reserved_output[v_req[4]])
        v_arbit = 0;
      if (v_arbit != 0) {
        grant4.write(1);
        v_select.range(14, 12) = v_req[4];
        v_free = v_free & (~v_arbit);
        v_connected_input[4] = 1;
        v_reserved_output[v_req[4]] = 1;
        if (req4.read()[5]) {
          v_connected_input[4] = 0;
          v_reserved_output[v_req[4]] = 0;
        }
      }
    }
    aselect.write(v_select);
  }
}
