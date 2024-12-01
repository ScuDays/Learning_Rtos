#include "tLib.h"

// 初始化节点
void tNodeInit(tNode *node)
{
	node->nextNode = node;
	node->preNode = node;
}

#define firstNode headNode.nextNode
#define lastNode headNode.preNode

// 初始化双向链表
void tListInit(tList *list)
{
	list->firstNode = &(list->headNode);
	list->lastNode = &(list->headNode);
	list->nodeCount = 0;
}

// 返回链表总节点数量
uint32_t tListCount(tList *list)
{
	return list->nodeCount;
}

// 获取链表第一个节点
tNode *tListFirst(tList *list)
{
	tNode *node = (tNode *)0;

	if (list->nodeCount != 0)
	{
		node = list->firstNode;
	}
	return node;
}

// 获取链表最后一个节点
tNode *tListLast(tList *list)
{
	tNode *node = (tNode *)0;

	if (list->nodeCount != 0)
	{
		node = list->lastNode;
	}
	return node;
}

// 获取链表特定节点前一个节点
tNode *tListPre(tList *list, tNode *node)
{
	if (node->preNode == node)
	{
		return (tNode *)0;
	}
	else
	{
		return node->preNode;
	}
}

// 获取链表特定节点后一个节点
tNode *tListNext(tList *list, tNode *node)
{
	if (node->nextNode == node)
	{
		return (tNode *)0;
	}
	else
	{
		return node->nextNode;
	}
}

// 在链表开头添加一个元素
void tListAddFirst(tList *list, tNode *node)
{
	node->preNode = list->firstNode->preNode;
	node->nextNode = list->firstNode;

	list->firstNode->preNode = node;
	list->firstNode = node;
	list->nodeCount++;
}

// 在链表末尾添加一个元素
void tListAddLast(tList *list, tNode *node)
{
	node->nextNode = &(list->headNode);
	node->preNode = list->lastNode;

	list->lastNode->nextNode = node;
	list->lastNode = node;
	list->nodeCount++;
}

// 在指定节点后面加入一个特定节点
void tListInsertAfter(tList *list, tNode *nodeAfter, tNode *nodeToInsert)
{
	nodeToInsert->preNode = nodeAfter;
	nodeToInsert->nextNode = nodeAfter->nextNode;

	nodeAfter->nextNode->preNode = nodeToInsert;
	nodeAfter->nextNode = nodeToInsert;

	list->nodeCount++;
}

// 删除链表第一个元素
tNode *tListRemoveFirst(tList *list)
{
	tNode *node = (tNode *)0;

	if (list->nodeCount != 0)
	{
		node = list->firstNode;

		node->nextNode->preNode = &(list->headNode);
		list->firstNode = node->nextNode;
		list->nodeCount--;
	}
	return node;
}
// 删除特定节点
void tListRemove(tList *list, tNode *node)
{
	node->preNode->nextNode = node->nextNode;
	node->nextNode->preNode = node->preNode;
	list->nodeCount--;
}

// 删除链表中的全部节点
void tListRemoveAll(tList *list)
{
	uint32_t count;
	tNode *nextNode;

	nextNode = list->firstNode;
	for (count = list->nodeCount; count != 0; count--)
	{
		tNode *currentNode = nextNode;
		nextNode = nextNode->nextNode;

		currentNode->nextNode = currentNode;
		currentNode->preNode = currentNode;
	}

	list->firstNode = &(list->headNode);
	list->lastNode = &(list->headNode);
	list->nodeCount = 0;
}
