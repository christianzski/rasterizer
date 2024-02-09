class rect {
	public:
		double x, y;
		double w, h;

		rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
};

class point {
	public:
		double x, y;
		point(double x, double y) : x(x), y(y) {}
};

class point3d {
	public:
		double x, y, z;
		point3d(double x, double y, double z) : x(x), y(y), z(z) {}

		void rotate_y(double angle) {
			double x = this->x * cos(angle) + 0 + this->z * sin(angle);
			double y = this->y;
			double z = this->x * -sin(angle) + 0 + this->z * cos(angle);

			this->x = x; this->y = y; this->z = z;
		}
};

point operator+(point a, point b) {
	return {a.x + b.x, a.y + b.y};
}

point3d operator+(point3d a, point3d b) {
	return {a.x + b.x, a.y + b.y, a.z + b.z};
}

point3d operator-(point3d a, point3d b) {
	return {a.x - b.x, a.y - b.y, a.z - b.z};
}

class triangle {
	public:
		point3d a, b, c;
		triangle(point3d a, point3d b, point3d c) : a(a), b(b), c(c) {}

		rect bounding_box() const {
			double xmin = std::min(a.x, std::min(b.x, c.x));
			double ymin = std::min(a.y, std::min(b.y, c.y));

			double xmax = std::max(a.x, std::max(b.x, c.x));
			double ymax = std::max(a.y, std::max(b.y, c.y));

			return rect(xmin, ymin, xmax - xmin + 1, ymax - ymin + 1);
		}

		// Return the beta and gamma coordinates of the given point
		point barycentric(point p) const {
			//double denominator = (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
			double denominator = b.x * (c.y - a.y) - a.x * c.y + c.x * (a.y - b.y) + a.x * b.y;
			//double beta = ((p.x - a.x) * (c.y - a.y) + (p.y - a.y) * -(c.x - a.x)) / denominator;
			double beta = p.x * (c.y - a.y) - a.x * c.y + p.y * (a.x - c.x) + a.y * c.x;
			beta /= denominator;


			//double gamma = ((p.x - a.x) * -(b.y - a.y) + (p.y - a.y) * (b.x - a.x)) / denominator;
			double gamma = p.x * (a.y - b.y) + a.x * b.y + p.y * (b.x - a.x) - a.y * b.x;
			gamma /= denominator;
			return point(beta, gamma);
		}

		// Translate the triangle position
		void translate(double x, double y, double z) {
			point3d delta(x, y, z);
			a = a + delta;
			b = b + delta;
			c = c + delta;
		}

		void scale(double scale) {
			a.x *= scale; a.y *= scale; a.z *= scale;
			b.x *= scale; b.y *= scale; b.z *= scale;
			c.x *= scale; c.y *= scale; c.z *= scale;
		}

		// Rotate the triangle along the y-axis
		void rotate(double angle) {
			a.rotate_y(angle);
			b.rotate_y(angle);
			c.rotate_y(angle);
		}

		// Compute the normal vector for the triangle using the determinant definition
		point3d compute_normal() {
			// Vector from a to b
			point3d ab = b - a;

			// Vector from a to c
			point3d ac = c - a;

			// Compute the cross product between ab and ac
			double cross_x = (ab.y * ac.z - ab.z * ac.y);
			double cross_y = (ab.z * ac.x - ab.x * ac.z);
			double cross_z = (ab.x * ac.y - ab.y * ac.x);

			// Normalize the vector
			double length = sqrt(cross_x * cross_x + cross_y * cross_y + cross_z * cross_z);

			return {cross_x / length, cross_y / length, cross_z / length};
		}
};
