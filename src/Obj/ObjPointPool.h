#ifndef OBJPOINTPOOL_H
#define OBJPOINTPOOL_H

class ObjPointPool {
public:
	ObjPointPool();
	~ObjPointPool();

	void	clear(int depth);	// Set zero points and number of floats per pt
	void	resize(int pts);	// Set a lot of pts

	int		accumulate(const float pt[]);	// Add a pt, extend if needed
	int		append(const float pt[]);		// Add a pt to the end
	void	set(int n, float pt[]);			// Set an existing pt
	
	int		count(void) const;
	float *	get(int index);
	const float *	get(int index) const;

private:

	vector<float>	mData;
	int				mDepth;

};

#endif
