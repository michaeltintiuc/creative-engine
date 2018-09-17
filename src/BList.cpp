#include "BList.h"

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/// BNode

BNode::BNode() : BBase() {}

BNode::~BNode() {}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/// BNodePri

BNodePri::BNodePri(TInt aPri) : BBase() { pri = aPri; }

BNodePri::~BNodePri() {}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/// BList

BList::BList() : BNode() { Reset(); }

BList::~BList() {
  //  if (next != this) printf("List not empty!");
}

void BList::AddTail(BNode &node) { node.InsertBeforeNode(this); }

void BList::AddHead(BNode &node) { node.InsertAfterNode(this); }

BNode *BList::RemHead() {
  BNode *n = next;
  if (n == (BNode *)this)
    return NULL;
  n->Remove();
  return n;
}

BNode *BList::RemTail() {
  BNode *n = prev;
  if (n == (BNode *)this)
    return NULL;
  n->Remove();
  return n;
}

void BList::RemoveNode(BNode *node) { node->Remove(); }

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/// BListPri

BListPri::BListPri() : BNodePri(0) { Reset(); }

BListPri::~BListPri() {
  //  if (next != this) printf("List not empty!");
}

void BListPri::AddTail(BNodePri &node) { node.InsertBeforeNode(this); }

void BListPri::AddHead(BNodePri &node) { node.InsertAfterNode(this); }

BNodePri *BListPri::RemHead() {
  BNodePri *n = next;
  if (n == (BNodePri *)this)
    return NULL;
  n->Remove();
  return n;
}

BNodePri *BListPri::RemTail() {
  BNodePri *n = prev;
  if (n == (BNodePri *)this)
    return NULL;
  n->Remove();
  return n;
}

void BListPri::RemoveNode(BNodePri *node) { node->Remove(); }

void BListPri::Add(BNodePri &node) {
  for (BNodePri *n = First(); !End(n); n = n->next) {
    if (node.pri >= n->pri) {
      node.InsertBeforeNode(n);
      return;
    }
  }
  AddTail(node);
}