

#pragma once







// namespace triangle_stripper

namespace triangle_stripper {









// FIFO Cache simulator

class cache_simulator

{

public:



	typedef unsigned int index;



	cache_simulator();



	void clear();

	void resize(const size_t Size);

	void reset();

	size_t size() const;



	void push(const index i, const bool CountCacheHit = false);



	void ResetHitCount();

	size_t HitCount() const;



protected:

	typedef std::deque<index> indices_deque;



	indices_deque	m_Cache;

	size_t			m_NbHits;

};









inline cache_simulator::cache_simulator() : m_NbHits(0) { }





inline void cache_simulator::clear() {

	m_Cache.clear();

}





inline void cache_simulator::resize(const size_t Size) {

	m_Cache.resize(Size, static_cast<index>(-1));

}





inline void cache_simulator::reset() {

	std::fill(m_Cache.begin(), m_Cache.end(), static_cast<index>(-1));

	ResetHitCount();

}





inline size_t cache_simulator::size() const {

	return m_Cache.size();

}





inline void cache_simulator::push(const index i, const bool CountCacheHit) {



	// Should we count the cache hits?

	if (CountCacheHit) {

		if (std::find(m_Cache.begin(), m_Cache.end(), i) != m_Cache.end())

			++m_NbHits;

	}

	    

	// Manage the indices cache as a FIFO structure

	m_Cache.push_front(i);

	m_Cache.pop_back();

}





inline void cache_simulator::ResetHitCount() {

	m_NbHits = 0;

}





inline size_t cache_simulator::HitCount() const {

	return m_NbHits;

}







} // namespace triangle_stripper





