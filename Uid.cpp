
#include <Log.h>
#include <malloc.h>
#include <Uid.h>
LinkedList<Uid*> Uid::_uids;

void Uid::debug(const char *s, Uid *p)
{
    INFO("%s '%s':%d at 0x%X \n", s, p->_label, p->_id, p);
}

Uid::Uid(const char *name)
{
    DEBUG(" new : label %s:0x%X\n", name, name);

    _label = (const char *)malloc(strlen(name) + 1);
    strcpy((char *)_label, name);

    _id = H(_label);
    _uids.add(this);
}

uid_t Uid::add(const char *label)
{
    Uid *uid = 0;
    uid = find(label);
    if (uid) {
        return uid->_id;
    }
    uid = new Uid(label);
    _uids.add(uid);
    return uid->_id;
}

Uid *Uid::find(const char *label)
{
    Uid* uid = _uids.findFirst([label](Uid* uid){
        return strcmp(uid->_label, label) == 0;
    });
    return uid;
}

uid_t Uid::id(const char *label)
{
    Uid *p = find(label);
    if (p)
        return p->_id;
    return 0;
}

Uid *Uid::find(uid_t id)
{
    Uid* uid = _uids.findFirst([id](Uid* uid){
        return uid->_id==id;
    });
    return uid;
}

const char *Uid::label(uid_t id)
{
    Uid *p = find(id);
    if (p)
        return p->_label;
    return "UNKNOWN";
}
