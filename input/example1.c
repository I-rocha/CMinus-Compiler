
int gcd(int u, int v)
{ if(v == 0) return u;
	else return gcd(v,u-u/v*v);
}

void main(void)
{	int x; int y;
	output(gcd(x,y));
}
