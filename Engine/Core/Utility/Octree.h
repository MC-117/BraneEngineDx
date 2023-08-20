#pragma once

#include "../Unit.h"

template<class V>
class Octree
{
public:
	struct Node
	{
		float width = 0;
		Vector3f center;
		Vector3f key;
		V* value = NULL;
		Node* nodes[8] = { NULL };

		int getIndex(const Vector3f& key) const;
		Vector3f getFactor(const Vector3f& key) const;
		Node* add(const Vector3f& key, const V& value);
		Node* add(Node* node);
		Node* find(const Vector3f& key);

		void releaseValue();
		void clear();
	};

	Octree() = default;
	virtual ~Octree() = default;

	bool empty() const;
	bool count() const;

	void add(const Vector3f& key, const V& value);
	bool remove(const Vector3f& key);
	void clear();

	V* find(const Vector3f& key) const;
	pair<Vector3f, V*> findClosest(const Vector3f& key) const;
protected:
	Node* root = NULL;
	size_t size = 0;
};

template<class V>
inline int Octree<V>::Node::getIndex(const Vector3f& key) const
{
	return ((center[0] < key[0]) * 1) | ((center[1] < key[1]) * 2) | ((center[2] < key[2]) * 4);
}

template<class V>
inline Vector3f Octree<V>::Node::getFactor(const Vector3f& key) const
{
	Vector3f ret = Vector3f(center[0] < key[0], center[1] < key[1], center[2] < key[2]);
	return ret * 2 - Vector3f::Ones();
}

template<class V>
inline Octree<V>::Node* Octree<V>::Node::add(const Vector3f& key, const V& value)
{
	if (this->key == key) {
		if (this->value == NULL)
			this->value = new V(value);
		else
			*this->value = value;
		return this;
	}

	int index = getIndex(key);
	float newWidth = width * 0.5f;

	if (center != this->key) {
		int curIndex = getIndex(this->key);
		Vector3f factor = getFactor(this->key);
		Node* node = new Node;
		node->width = newWidth;
		node->center = center + factor * newWidth;
		node->key = key;
		node->value = this->value;
		this->value = NULL;
		nodes[curIndex] = node;
		if (curIndex == index)
			return node->add(key, value);
	}

	Vector3f factor = getFactor(key);
	Node* node = new Node;
	node->width = newWidth;
	node->center = center + factor * newWidth;
	node->key = key;
	node->value = new V(value);
	nodes[index] = node;
	return node;
}

template<class V>
inline Octree<V>::Node* Octree<V>::Node::add(Node* node)
{
	int index = getIndex(node->key);
	Vector3f factor = getFactor(node->key);
	node->width = width * 0.5f;
	node->center = center + factor * node->width;
	nodes[index] = node;
	return node;
}

template<class V>
inline Octree<V>::Node* Octree<V>::Node::find(const Vector3f& key)
{
	if (this->key == key)
		return this;
	return nodes[getIndex(key)];
}

template<class V>
inline void Octree<V>::Node::releaseValue()
{
	if (value != NULL) {
		delete value;
		value = NULL;
	}
}

template<class V>
inline void Octree<V>::Node::clear()
{
	for (int i = 0; i < 8; i++) {
		Node*& node = nodes[i];
		if (node != NULL) {
			node->releaseValue();
			delete node;
			node = NULL;
		}
	}
}

template<class V>
inline bool Octree<V>::empty() const
{
	return size == 0;
}

template<class V>
inline bool Octree<V>::count() const
{
	return size;
}

template<class V>
inline void Octree<V>::add(const Vector3f& key, const V& value)
{
	size++;
	if (root == NULL) {
		root = new Node();
		root->used = true;
		root->key = key;
		root->center = key;
		root->value = value;
	}
	else {
		Vector3f offset = key - root->key;
		float width = max(offset.x(), max(offset.y(), offset.z()));
		if (root->width < width) {
			Vector3f center = (key + root->key) * 0.5f;
			Node* newRoot = new Node;
			newRoot->width = width;
			newRoot->used = true;
			newRoot->key = center;
			newRoot->center = center;
			newRoot->add(root);
			root = newRoot;
			newRoot->add(key, value);
		}
		else {
			Node* node = root;
			while (true) {
				Node* _node = node->find(key);
				if (_node == NULL || _node == node)
					break;
				node = _node;
			}
			node->add(key, value);
		}
	}
}

template<class V>
inline bool Octree<V>::remove(const Vector3f& key)
{
	if (root == NULL)
		return false;
	Node* node = root->find(key);
	if (node == NULL)
		return false;
	node->releaseValue();
	size--;
	return true;
}

template<class V>
inline void Octree<V>::clear()
{
	if (root == NULL)
		return;
	root->clear();
	root->releaseValue();
	delete root;
	root = NULL;
	size = 0;
}

template<class V>
inline V* Octree<V>::find(const Vector3f& key) const
{
	if (root == NULL)
		return NULL;
	Node* node = root;
	while (true) {
		Node* _node = node->find(key);
		if (_node == NULL)
			return NULL;
		else if (_node == node)
			return node->value;
		node = _node;
	}
}
