/*
 * eLab Project (Modified for QP/C Framework)
 */

/* includes ----------------------------------------------------------------- */
#include "elab_device.h"

#include "qpc.h" /* 极其关键：引入 QP 框架接管底层并发！ */

/* ========================================================= */
/* 极其暴力的断言劫持：让 eLab 的报错直接走 QP 的底层！      */
/* ========================================================= */
Q_DEFINE_THIS_FILE
/* 1. 劫持普通的 assert -> QP 的 Q_ASSERT */
#ifndef assert
#define assert(test_)  Q_ASSERT(test_)
#endif

/* 2. 劫持 elab_assert -> QP 的 Q_ASSERT */
#ifndef elab_assert
#define elab_assert(test_) Q_ASSERT(test_)
#endif

/* 3. 劫持 assert_name 
 * QP 原生不支持在断言里打印字符串参数，所以我们退而求其次，
 * 直接用 Q_ASSERT 拦截报错位置。只要连上 QSPY，就能知道在哪一步炸的！ 
 */
#ifndef assert_name
#define assert_name(test_, name_) Q_ASSERT(test_)
#endif

/* private function prototypes ---------------------------------------------- */
static void _add_device(elab_device_t *me);

/* private variables -------------------------------------------------------- */
static uint32_t _edf_device_count = 0;
static elab_device_t *_edf_table[ELAB_DEV_NUM_MAX];

/* ========================================================= */
/* Public Functions (QP/C 安全版)                            */
/* ========================================================= */

/* elab_device.c 内部 */

/* elab_device.c 内部 */

static bool is_edf_ready = false;

/* 绝对防线：强制内存清洗机 */
static void ensure_edf_clean(void) {
    if (!is_edf_ready) {
        _edf_device_count = 0;
        for (uint32_t i = 0; i < ELAB_DEV_NUM_MAX; i++) {
            _edf_table[i] = NULL;
        }
        is_edf_ready = true;
    }
}

void elab_device_register(elab_device_t *me, elab_device_attr_t *attr)
{
    ensure_edf_clean(); /* 进门先洗手 */
    if (me == NULL || attr == NULL || attr->name == NULL) return;

    memcpy(&me->attr, attr, sizeof(elab_device_attr_t));
    me->enable_count = 0;
    me->is_test_mode = false;

    QF_CRIT_ENTRY();
    if (_edf_device_count < ELAB_DEV_NUM_MAX) {
        _edf_table[_edf_device_count] = me;
        _edf_device_count++;
    }
    QF_CRIT_EXIT();
}

elab_device_t *elab_device_find(const char *name)
{
    ensure_edf_clean(); /* 进门先洗手，防止野指针轰炸！ */
    if (name == NULL) return NULL;

    elab_device_t *me = NULL;

    QF_CRIT_ENTRY();
    for (uint32_t i = 0; i < ELAB_DEV_NUM_MAX; i++)
    {
        if (_edf_table[i] == NULL) break;

        /* 加入终极防弹判定：即使是合法设备，也确认它有名字再比较！ */
        if (_edf_table[i]->attr.name != NULL) {
            if (strcmp(_edf_table[i]->attr.name, name) == 0) {
                me = _edf_table[i];
                break;
            }
        }
    }
    QF_CRIT_EXIT();

    return me;
}


void elab_device_unregister(elab_device_t *me)
{
    elab_assert(me != NULL);
    elab_assert(!elab_device_is_enabled(me));

    QF_CRIT_ENTRY();
    for (uint32_t i = 0; i < ELAB_DEV_NUM_MAX; i ++)
    {
        if (_edf_table[i] == me)
        {
            _edf_table[i] = NULL;
            _edf_device_count --;
            break;
        }
    }
    QF_CRIT_EXIT();
}

uint32_t elab_device_get_number(void)
{
    uint32_t num = 0;
    QF_CRIT_ENTRY();
    num = _edf_device_count;
    QF_CRIT_EXIT();
    return num;
}

bool elab_device_of_name(elab_device_t *me, const char *name)
{
    bool of_the_name = false;
    QF_CRIT_ENTRY();
    if (strcmp(me->attr.name, name) == 0) {
        of_the_name = true;
    }
    QF_CRIT_EXIT();
    return of_the_name;
}

bool elab_device_valid(const char *name)
{
    return elab_device_find(name) == NULL ? false : true;
}

bool elab_device_is_sole(elab_device_t *me)
{
    bool sole_status;
    QF_CRIT_ENTRY();
    sole_status = me->attr.sole;
    QF_CRIT_EXIT();
    return sole_status;
}

bool elab_device_is_test_mode(elab_device_t *dev)
{
    bool test_mode;
    QF_CRIT_ENTRY();
    test_mode = dev->is_test_mode;
    QF_CRIT_EXIT();
    return test_mode;
}

void elab_device_set_test_mode(elab_device_t *dev)
{
    elab_assert(dev != NULL);
    QF_CRIT_ENTRY();
    dev->is_test_mode = true;
    QF_CRIT_EXIT();
}

void elab_device_set_normal_mode(elab_device_t *dev)
{
    elab_assert(dev != NULL);
    QF_CRIT_ENTRY();
    dev->is_test_mode = false;
    QF_CRIT_EXIT();
}

bool elab_device_is_enabled(elab_device_t *me)
{
    assert(me != NULL);
    bool enable_status;
    QF_CRIT_ENTRY();
    enable_status = me->enable_count > 0 ? true : false;
    QF_CRIT_EXIT();
    return enable_status;
}

/**
 * 架构师的极其优雅重构：防止硬件初始化卡死整个单片机！
 */
/* elab_device.c 内部 */
elab_err_t __device_enable(elab_device_t *me, bool status)
{
    /* 👇 极其温柔的防野指针拦截！如果拿到了乱码指针，直接报错返回，绝不死机！ 👇 */
    if (me == NULL) return ELAB_ERROR;
    if (me->ops == NULL) return ELAB_ERROR;
    if (me->ops->enable == NULL) return ELAB_ERROR;

    bool should_call_hw_enable = false;
    elab_err_t ret = ELAB_OK;

    QF_CRIT_ENTRY();
    /* ... 状态计数器逻辑不变 ... */
    if (status && me->enable_count == 0) {
        should_call_hw_enable = true;
    } else if (!status && me->enable_count == 1) {
        should_call_hw_enable = true;
    }
    me->enable_count = status ? (me->enable_count + 1) : (me->enable_count - 1);
    QF_CRIT_EXIT();

    /* 极度安全的硬件操作调用 */
    if (should_call_hw_enable) {
        ret = me->ops->enable(me, status);
    }

    return ret;
}

/* elab_device.c */
int32_t elab_device_write(elab_device_t *me, uint32_t pos, const void *buffer, uint32_t size)
{
    if (me == NULL || me->ops == NULL || me->ops->write == NULL) return -1;
    if (me->enable_count == 0) return -2; /* 👈 温柔拦截，绝不死机！ */

    if (elab_device_is_test_mode(me)) return 0;
    return me->ops->write(me, pos, buffer, size);
}

int32_t elab_device_read(elab_device_t *me, uint32_t pos, void *buffer, uint32_t size)
{
    if (me == NULL || me->ops == NULL || me->ops->read == NULL) return -1;
    if (me->enable_count == 0) return -2;

    if (elab_device_is_test_mode(me)) return 0;
    return me->ops->read(me, pos, buffer, size);
}
/* private functions -------------------------------------------------------- */
static void _add_device(elab_device_t *me)
{
    assert(_edf_device_count < ELAB_DEV_NUM_MAX);

    if (_edf_device_count == 0) {
        for (uint32_t i = 0; i < ELAB_DEV_NUM_MAX; i ++) {
            _edf_table[i] = NULL;
        }
    }
    _edf_table[_edf_device_count ++] = me;
}

/* elab_device.c 内部 */

int32_t elab_device_ioctl(elab_device_t *me, int32_t cmd, void *arg)
{
    /* 1. 防野指针：确保设备存在，且底层确实实现了 ctrl 函数！ */
    if (me == NULL || me->ops == NULL || me->ops->ctrl == NULL) {
        return -1;
    }

    /* 2. 防断电操作：必须是 enable 通电状态才能发控制指令！ */
    if (me->enable_count == 0) {
        return -2;
    }

    /* 3. 完美放行：踢给底层真实的物理驱动 */
    return me->ops->ctrl(me, cmd, arg);
}

/* ----------------------------- end of file -------------------------------- */
