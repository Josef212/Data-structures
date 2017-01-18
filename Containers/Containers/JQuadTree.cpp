#include "JQuadTree.h"
#include "GameObject.h"

#define MAX_NODE_OBJECTS 8


//---------------------------------------------------
//---------------TreeNode----------------------------
//---------------------------------------------------


treeNode::treeNode(const AABB& _box) : box(_box)
{
	parent = nullptr;
	for (unsigned int i = 0; i < 4; ++i)
		childs[i] = nullptr;
}

treeNode::~treeNode()
{
}

void treeNode::insert(GameObject* obj)
{
	if (!obj)
		return;

	if (childs[0] == nullptr && objects.size() < MAX_NODE_OBJECTS)
		objects.push_back(obj);
	else
	{
		if (childs[0] == nullptr)
			divideNode();

		objects.push_back(obj);
		ajustNode();
	}
}

void treeNode::erase(GameObject* obj)
{
	std::list<GameObject*>::iterator tmp = std::find(objects.begin(), objects.end(), obj);
	if (tmp != objects.end())
		objects.erase(tmp);

	if (childs[0] != nullptr)
		for (unsigned int i = 0; i < 4; ++i)
			if (childs[i])childs[i]->erase(obj);
}

void treeNode::coollectBoxes(std::vector<AABB>& vec)
{
	for (std::list<GameObject*>::iterator it = objects.begin(); it != objects.end(); ++it)
	{
		vec.push_back((*it)->aabb);
	}

	for (unsigned int i = 0; i < 4; ++i)
		if (childs[i] != nullptr)
			childs[i]->coollectBoxes(vec);
}

void treeNode::coollectGO(std::vector<GameObject*>& vec)
{
	for (std::list<GameObject*>::iterator it = objects.begin(); it != objects.end(); ++it)
	{
		vec.push_back((*it));
	}

	for (unsigned int i = 0; i < 4; ++i)
		if (childs[i] != nullptr)
			childs[i]->coollectGO(vec);
}

void treeNode::divideNode()
{
	float3 center = box.CenterPoint();
	float3 center2 = float3::zero;
	float3 size = box.Size();
	float3 size2(size.x * 0.5f, size.y, size.z * 0.5f);
	AABB tmp;

	float sx = size.x * 0.25f;
	float sz = size.z * 0.25f;

	//----------Top-right
	center2.Set(center.x + sx, center.y, center.z + sz);
	tmp.SetFromCenterAndSize(center2, size2);
	childs[0] = new treeNode(tmp);

	//----------Bottom-right
	center2.Set(center.x + sx, center.y, center.z - sz);
	tmp.SetFromCenterAndSize(center2, size2);
	childs[1] = new treeNode(tmp);

	//----------Bottom-left
	center2.Set(center.x - sx, center.y, center.z - sz);
	tmp.SetFromCenterAndSize(center2, size2);
	childs[2] = new treeNode(tmp);

	//----------Top-right
	center2.Set(center.x - sx, center.y, center.z + sz);
	tmp.SetFromCenterAndSize(center2, size2);
	childs[3] = new treeNode(tmp);

	for (unsigned int i = 0; i < 4; ++i)
		childs[i]->parent = this;
}

void treeNode::ajustNode()
{
	std::list<GameObject*>::iterator it = objects.begin();
	while (it != objects.end())
	{
		GameObject* tmp = (*it);

		bool intersections[4];
		for (unsigned int i = 0; i < 4; ++i)
			intersections[i] = childs[i]->box.Intersects(tmp->aabb);

		if (intersections[0] && intersections[1] && intersections[2] && intersections[3])
			++it; //Let the object in parent if it intersects with all childs
		else
		{
			it = objects.erase(it);
			for (unsigned int i = 0; i < 4; ++i)
				if (childs[i]->box.Intersects(tmp->aabb)) //box.MinimalEnclosingAABB().Intersects()
					childs[i]->insert(tmp);
		}
	}
}

bool treeNode::intersectsAllChilds(const AABB& _box)
{
	unsigned int count = 0;

	for (unsigned int i = 0; i < 4; ++i)
		if (childs[i]->box.Intersects(_box)) //box.MinimalEnclosingAABB().Intersects()
			++count;

	return count == 4;
}

/*template<class TYPE>
void treeNode::collectCandidates(std::vector<GameObject*>& vec, const TYPE& primitive)
{
	if (primitive.Intersects(box))
		for (std::vector<GameObject*>::iterator it = objects.begin(); it != objects.end(); ++it)
			if (primitive.Intersects((*it)->aabb))
				vec.push_back((*it));

	for (unsigned int i = 0; i < 4; ++i)
		if (childs[i])childs[i]->collectCandidates(vec, primitive);
}*/

void treeNode::collectCandidates(std::vector<GameObject*>& vec, const Frustum& frustum)
{
	if (frustum.Intersects(box))
		for (std::list<GameObject*>::iterator it = objects.begin(); it != objects.end(); ++it)
			if (frustum.Intersects((*it)->aabb))
				vec.push_back((*it));

	for (unsigned int i = 0; i < 4; ++i)
		if (childs[i])childs[i]->collectCandidates(vec, frustum);
}

//---------------------------------------------------
//---------------JQuadTree---------------------------
//---------------------------------------------------


JQuadTree::JQuadTree()
{
}


JQuadTree::~JQuadTree()
{
	clear();
}

void JQuadTree::insert(GameObject* obj)
{
	if (rootNode && obj)
		if (rootNode->box.Intersects(obj->aabb))
			rootNode->insert(obj);
}

void JQuadTree::erase(GameObject* obj)
{
	if (rootNode && obj)
		rootNode->insert(obj);
}

void JQuadTree::setRoot(const AABB& _box)
{
	if (rootNode)
		delete(rootNode);

	rootNode = new treeNode(_box);
}

void JQuadTree::clear()
{

	if (rootNode)
		delete(rootNode);
	rootNode = nullptr;
}

/*template<class TYPE>
void JQuadTree::collectCandidates(std::vector<GameObject*>& vec, const TYPE& primitive)
{
	if (rootNode)
		if (primitive.Intersects(rootNode->box))
			rootNode->collectCandidates(vec, primitive);
}*/

void JQuadTree::collectCandidates(std::vector<GameObject*>& vec, const Frustum& frustum)
{
	if (rootNode)
		if (frustum.Intersects(rootNode->box))
			rootNode->collectCandidates(vec, frustum);
}