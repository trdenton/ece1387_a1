#ifndef __CIRCUIT_H__
#define __CIRCUIT_H__
class connection {
	public:
		int x0 = 0, y0 = 0, p0 = 0, x1 = 0, y1 = 0, p1 = 0;
		connection(int x_0, int y_0, int p_0, int x_1, int y_1, int p_1) {
			x0 = x_0;
			y0 = y_0;
			p0 = p_0;
			x1 = x_1;
			y1 = y_1;
			p1 = p_1;
		}
};

class circuit {
	public:
		int grid_size, tracks_per_channel;

		circuit(int gs, int tpc) {
			grid_size = gs;
			tracks_per_channel = tpc;
		}

		void add_connection(connection* conn) {
		}
};
#endif
