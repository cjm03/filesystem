#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tree.h"

static char* SafeStrdup(const char* str) {
    if (!str) return NULL;
    size_t size = strlen(str) + 1;
    char* new = malloc(size);
    if (!new) return NULL;
    memcpy(new, str, size);
    return new;
}

static FSNode* NewNode(const char* name, FSNodeType type) {
    FSNode* n = calloc(1, sizeof(FSNode));
    if (!n) return NULL;
    n->name = SafeStrdup(name ? name : "");
    n->type = type;
    n->content = NULL;
    n->parent = n->child = n->next = NULL;
    return n;
}

static int UnlinkFromParent(FSNode* node) {
    if (!node || !node->parent) return -1;
    FSNode* p = node->parent;
    FSNode* prev = NULL;
    FSNode* cur = p->child;
    while (cur) {
        if (cur == node) {
            if (prev) prev->next = cur->next;
            else p->child = cur->next;
            cur->next = NULL;
            cur->parent = NULL;
            return 0;
        }
        prev = cur;
        cur = cur->next;
    }
    return -1;
}


void FSFree(FSNode* root) {
    if (!root) return;

    if (root->parent) UnlinkFromParent(root);
    size_t cap = 16;
    size_t top = 0;
    FSNode** stack = malloc(cap * sizeof(FSNode*));
    if (!stack) {
        FSNode* c = root->child;
        while (c) {
            FSNode* next = c->next;
            FSFree(c);
            c = next;
        }
        free(root->name);
        free(root->content);
        free(root);
    }
    stack[top++] = root;
    while (top > 0) {
        FSNode* node = stack[--top];
        for (FSNode* c = node->child; c; c = c->next) {
            if (top == cap) {
                size_t newcap = cap * 2;
                FSNode** newstack = realloc(stack, newcap * sizeof(FSNode*));
                if (!newstack) {
                    for (FSNode* cc = c; cc; cc = cc->next) FSFree(cc);
                    break;
                }
                stack = newstack;
                cap = newcap;
            }
            stack[top++] = c;
        }
        free(node->name);
        free(node->content);
        free(node);
    }
    free(stack);
}

FSNode* FSNewRoot(void) {
    FSNode* r = NewNode("", FS_DIR);
    return r;
}

static char** SplitPath(const char* path, int* count) {
    if (!path || path[0] != '/') {
        *count = 0;
        return NULL;
    }
    char* copy = SafeStrdup(path);
    if (!copy) { 
        *count = 0; 
        return NULL; 
    }
    int cnt = 0;
    char* p = copy;
    while (*p) {
        if (*p == '/') {
            p++;
            continue;
        }
        cnt++;
        while (*p && *p != '/') p++;
    }
    char** parts = calloc((cnt + 1), sizeof(char*));
    if (!parts) {
        free(copy);
        *count = 0;
        return NULL;
    }
    int i = 0;
    char* saveptr = NULL;
    char* tok = strtok_r(copy, "/", &saveptr);
    while (tok) {
        parts[i++] = SafeStrdup(tok);
        tok = strtok_r(NULL, "/", &saveptr);
    }
    free(copy);
    *count = cnt;
    return parts;
}

static void FreeParts(char** parts, int count) {
    if (!parts) return;
    for (int i = 0; i < count; ++i) free(parts[i]);
    free(parts);
}

static FSNode* FindChild(const FSNode* parent, const char* name) {
    if (!parent || parent->type != FS_DIR) return NULL;
    FSNode* c = parent->child;
    while (c) {
        if (strcmp(c->name, name) == 0) return (FSNode*)c;
        c = c->next;
    }
    return NULL;
}

static int AddChild(FSNode* parent, FSNode* child) {
    if (!parent || parent->type != FS_DIR || !child) return -1;
    if (FindChild(parent, child->name)) return -1; // already exists
    child->next = parent->child;
    parent->child = child;
    child->parent = parent;
    return 0;
}

static FSNode* Traverse(FSNode* root, const char* path, int create_dirs) {
    if (!root || !path || path[0] != '/') return NULL;
    if (strcmp(path, "/") == 0) return root;
    int count = 0;
    char **parts = SplitPath(path, &count);
    if (!parts && count != 0) return NULL;
    FSNode *cursor = root;
    for (int i = 0; i < count; ++i) {
        FSNode *next = FindChild(cursor, parts[i]);
        if (!next) {
            if (create_dirs) {
                /* create directory */
                FSNode *d = NewNode(parts[i], FS_DIR);
                if (!d) { FreeParts(parts, count); return NULL; }
                AddChild(cursor, d);
                next = d;
            } else {
                FreeParts(parts, count);
                return NULL;
            }
        }
        cursor = next;
    }
    FreeParts(parts, count);
    return cursor;
}

FSNode* FSFind(FSNode* root, const char* path) {
    return Traverse(root, path, 0);
}

FSNode* FSMkdir(FSNode* root, const char* path) {
    if (!root || !path) return NULL;
    if (strcmp(path, "/") == 0) return root;
    return Traverse(root, path, 1);
}

FSNode *FSCreateFile(FSNode *root, const char *path, const char *content) {
    if (!root || !path || path[0] != '/') return NULL;
    if (strcmp(path, "/") == 0) return NULL; // cannot create root as file
    // split path to parent and final name
    char *copy = SafeStrdup(path);
    if (!copy) return NULL;
    char *last_slash = strrchr(copy, '/');
    assert(last_slash);
    *last_slash = '\0';
    const char *parent_path = (*copy == '\0') ? "/" : copy;
    const char *fname = last_slash + 1;
    if (fname[0] == '\0') { free(copy); return NULL; }
    FSNode *parent = Traverse(root, parent_path, 1); // create intermediate dirs
    if (!parent) { free(copy); return NULL; }
    FSNode *existing = FindChild(parent, fname);
    if (existing) {
        if (existing->type != FS_FILE) {
            free(copy);
            return NULL; // cannot overwrite dir with file
        }
        // overwrite content
        free(existing->content);
        existing->content = SafeStrdup(content);
        free(copy);
        return existing;
    }
    FSNode *file = NewNode(fname, FS_FILE);
    if (!file) { free(copy); return NULL; }
    file->content = SafeStrdup(content);
    if (AddChild(parent, file) != 0) {
        FSFree(file);
        free(copy);
        return NULL;
    }
    free(copy);
    return file;
}

int FSRemove(FSNode *root, const char *path) {
    if (!root || !path || path[0] != '/') return -1;
    if (strcmp(path, "/") == 0) return -1; // cannot remove root
    FSNode *node = FSFind(root, path);
    if (!node) return -1;
    if (UnlinkFromParent(node) != 0) return -1;
    FSFree(node);
    return 0;
}

int FSMove(FSNode *root, const char *src_path, const char *dest_dir_path) {
    if (!root || !src_path || !dest_dir_path) return -1;
    if (strcmp(src_path, "/") == 0) return -1;
    FSNode *src = FSFind(root, src_path);
    if (!src) return -1;
    FSNode *dest = FSFind(root, dest_dir_path);
    if (!dest) return -1;
    if (dest->type != FS_DIR) return -1;
    // cannot move dest into its own subtree
    FSNode *p = dest;
    while (p) {
        if (p == src) return -1;
        p = p->parent;
    }
    // if child with same name exists in dest -> error
    if (FindChild(dest, src->name)) return -1;
    if (UnlinkFromParent(src) != 0) return -1;
    if (AddChild(dest, src) != 0) return -1;
    return 0;
}

const char *FSGetFileContent(FSNode *root, const char *path) {
    FSNode *n = FSFind(root, path);
    if (!n || n->type != FS_FILE) return NULL;
    return n->content;
}

/* Print helpers */
static void PrintIndent(int depth) {
    for (int i = 0; i < depth; ++i) printf("    ");
}

static void PrintTreeRec(const FSNode *node, int depth) {
    if (!node) return;
    // don't print root name as empty string; print "/"
    if (node->parent == NULL) {
        printf("/\n");
    } else {
        PrintIndent(depth);
        printf("%s", node->name);
        if (node->type == FS_DIR) printf("/\n");
        else {
            printf("  (file");
            if (node->content) printf(", %zu bytes", strlen(node->content));
            printf(")\n");
        }
    }
    if (node->type == FS_DIR) {
        /* iterate children in sorted-ish order? Current structure is un-ordered (prepend). We'll print as-is. */
        FSNode *c = node->child;
        while (c) {
            PrintTreeRec(c, depth + (node->parent ? 1 : 0));
            c = c->next;
        }
    }
}

void FSPrintTree(const FSNode *node) {
    PrintTreeRec(node, 0);
}

void PrintLS(FSNode* node) {
    if (node->type == FS_DIR) {
        FSNode* c = node->child;
        while (c) {
            if (c->type == FS_DIR) printf("%s/ ", c->name);
            else printf("%s ", c->name);
            c = c->next;
        }
        printf("\n");
    } else {
        printf("cannot ls a file\n");
    }
}
