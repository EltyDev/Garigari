#include "LiveActor/RailRider.hpp"
#include "Util/MathUtil.hpp"
#include "Util/SceneUtil.hpp"


RailRider::RailRider(const JMapInfoIter &rIter) {
    mBezierRail = nullptr;
    mCoord = 0.0f;
    mSpeed = 0.0f;
    mIsNotReverse = true;
    mCurPos.x = 0.0f;
    mCurPos.y = 0.0f;
    mCurPos.z = 0.0f;
    mCurDirection.x = 1.0f;
    mCurDirection.y = 0.0f;
    mCurDirection.z = 0.0f;
    mStartPos.x = 0.0f;
    mStartPos.y = 0.0f;
    mStartPos.z = 0.0f;
    mEndPos.x = 0.0f;
    mEndPos.y = 0.0f;
    mEndPos.z = 0.0f;
    const JMapInfo* info = nullptr;
    JMapInfoIter iter(nullptr, -1);
    MR::getRailInfo(&iter, &info, rIter);
    initBezierRail(iter, info);
}

RailRider::RailRider(s32 a1, s32 a2) {
    mBezierRail = nullptr;
    mCoord = 0.0f;
    mSpeed = 0.0f;
    mIsNotReverse = true;
    mCurPos.x = 0.0f;
    mCurPos.y = 0.0f;
    mCurPos.z = 0.0f;
    mCurDirection.x = 1.0f;
    mCurDirection.y = 0.0f;
    mCurDirection.z = 0.0f;
    mStartPos.x = 0.0f;
    mStartPos.y = 0.0f;
    mStartPos.z = 0.0f;
    mEndPos.x = 0.0f;
    mEndPos.y = 0.0f;
    mEndPos.z = 0.0f;
    const JMapInfo* info = nullptr;
    JMapInfoIter iter(nullptr, -1);
    MR::getCameraRailInfo(&iter, &info, a1, a2);
    initBezierRail(iter, info);
}

void RailRider::move() {
    if (mIsNotReverse) {
        mCoord += mSpeed;
    }
    else {
        mCoord -= mSpeed;
    }

    mCoord = mBezierRail->normalizePos(mCoord, 1);
    syncPosDir();
}

void RailRider::moveToNearestPos(const TVec3f &rPos) {
    mCoord = mBezierRail->getNearestRailPosCoord(rPos);
    syncPosDir();
}

void RailRider::moveToNearestPoint(const TVec3f& rPoint) {
    float minDist = FLT_MAX;
    int closestIndex = 0;

    for (int i = 0; i < (int)mBezierRail->mPointNum; ++i) {
        TVec3f point;
        copyPointPos(&point, i);
        //WIP Implement and using PSVECDotProduct
        float dx = rPoint.x - point.x;
        float dy = rPoint.y - point.y;
        float dist = dx * dx + dy * dy;

        if (dist < minDist) {
            minDist = dist;
            closestIndex = i;
        }
    }

    mCoord = mBezierRail->getRailPosCoord(closestIndex);
    syncPosDir();
}

void RailRider::moveToNextPoint() {
    mCoord = mBezierRail->getRailPosCoord(getNextPointNo());
    syncPosDir();
}

void RailRider::reverse() {
    if (mIsNotReverse != false) {
        mIsNotReverse = false;
    }
    else {
        mIsNotReverse = true;
    }

    syncPosDir();
}

void RailRider::calcPosAtCoord(TVec3f *pOutVec, f32 a2) const {
    mBezierRail->calcPos(pOutVec, a2);
}

void RailRider::calcDirectionAtCoord(TVec3f *pOutVec, f32 a2) const {
    mBezierRail->calcDirection(pOutVec, a2);
}

f32 RailRider::calcNearestPos(const TVec3f &rPos) const {
    return mBezierRail->getNearestRailPosCoord(rPos);
}

f32 RailRider::getTotalLength() const {
    return mBezierRail->getTotalLength();
}

f32 RailRider::getPartLength(int idx) const {
    return mBezierRail->getPartLength(idx);
}

bool RailRider::isLoop() const {
    return mBezierRail->mIsClosed;
}

/* https://decomp.me/scratch/byVJ8 */
bool RailRider::isReachedGoal() const {
    if (mBezierRail->mIsClosed) {
        return false;
    }

    bool v3 = false;
    bool v4 = true;

    if (mIsNotReverse) {
        if (MR::isNearZero(mCoord - mBezierRail->getTotalLength(), 0.0f)) {
            v4 = true;
        }
    }

    if (!v4) {
        bool v6 = false;
        if (!mIsNotReverse && MR::isNearZero(mCoord, 0.001f)) {
            v6 = true;
        }

        if (!v6) {
            v3 = false;
        }
    }

    return v3;
}

bool RailRider::isReachedEdge() const {
    bool ret;

    if (mBezierRail->mIsClosed) {
        return false;
    }
    else {
        ret = true;

        if (!MR::isNearZero(mCoord, 0.001f)) {
            f32 val = mCoord - mBezierRail->getTotalLength();
            if (!MR::isNearZero(val, 0.001f)) {
                ret = false;
            }
        }
    }

    return ret;
}

void RailRider::setCoord(f32 coord) {
    mCoord = coord;
    mCoord = mBezierRail->normalizePos((f64)coord, 1);
    syncPosDir();
}

void RailRider::setSpeed(f32 coord) {
    mSpeed = coord;
}

bool RailRider::getRailArgWithInit(const char *pStr, s32 *pOut) const {
    s32 val;
    if (!mBezierRail->mIter->getValue<s32>(pStr, &val)) {
        return false;
    }

    *pOut = val;
    return true;
}

bool RailRider::getRailArgNoInit(const char *pStr, s32 *pOut) const {
    s32 val;
    if (!mBezierRail->mIter->getValue<s32>(pStr, &val)) {
        return false;
    }

    if (val == -1) {
        return false;
    }

    *pOut = val;
    return true;
}

f32 RailRider::getNextPointCoord() const {
    return mBezierRail->getRailPosCoord(getNextPointNo());
}

f32 RailRider::getCurrentPointCoord() const {
    return mBezierRail->getRailPosCoord(mCurPoint);
}

s32 RailRider::getPointNum() const {
    return mBezierRail->mPointNum;
}

void RailRider::copyPointPos(TVec3f *pOut, s32 pointNum) const {
    JMapInfoIter iter;
    iter.mInfo = nullptr;
    iter.mIndex = -1;
    mBezierRail->calcRailCtrlPointIter(&iter, pointNum);
    MR::getRailPointPos0(iter, pOut);
}

f32 RailRider::getPointCoord(s32 idx) const {
    return mBezierRail->getRailPosCoord(idx);
}

void RailRider::initBezierRail(const JMapInfoIter &rIter, const JMapInfo *pInfo) {
    mBezierRail = new BezierRail(rIter, pInfo);
    syncPosDir();
    setCoord(mBezierRail->getTotalLength());
    mEndPos.set<f32>(mCurPos);
    setCoord(0.0f);
    mStartPos.set<f32>(mCurPos);
}

bool RailRider::getPointArgS32NoInit(const char *pStr, s32 *pOut, s32 pointNum) const {
    s32 val;
    JMapInfoIter iter(nullptr, -1);

    mBezierRail->calcRailCtrlPointIter(&iter, pointNum);
    val = -1;
    iter.getValue<s32>(pStr, &val);

    if (val != -1) {
        *pOut = val;
        return true;
    }

    return false;
}

bool RailRider::getPointArgS32WithInit(const char *pStr, s32 *pOut, s32 pointNum) const {
    JMapInfoIter iter(nullptr, -1);
    mBezierRail->calcRailCtrlPointIter(&iter, pointNum);
    iter.getValue<s32>(pStr, pOut);
    return true;
}

bool RailRider::getCurrentPointArgS32NoInit(const char *pStr, s32 *pOut) const {
    s32 val;
    JMapInfoIter iter(nullptr, -1);

    mBezierRail->calcRailCtrlPointIter(&iter, mCurPoint);
    val = -1;
    iter.getValue<s32>(pStr, &val);

    if (val != -1) {
        *pOut = val;
        return true;
    }

    return false;
}

bool RailRider::getCurrentPointArgS32WithInit(const char *pStr, s32 *pOut) const {
    JMapInfoIter iter(nullptr, -1);
    mBezierRail->calcRailCtrlPointIter(&iter, mCurPoint);
    iter.getValue<s32>(pStr, pOut);
    return true;
}

bool RailRider::getNextPointArgS32NoInit(const char *pStr, s32 *pOut) const {
    s32 val;
    JMapInfoIter iter(nullptr, -1);

    mBezierRail->calcRailCtrlPointIter(&iter, getNextPointNo());
    val = -1;
    iter.getValue<s32>(pStr, &val);

    if (val != -1) {
        *pOut = val;
        return true;
    }

    return false;
}

bool RailRider::getNextPointArgS32WithInit(const char *pStr, s32 *pOut) const {
    JMapInfoIter iter(nullptr, -1);
    mBezierRail->calcRailCtrlPointIter(&iter, getNextPointNo());
    iter.getValue<s32>(pStr, pOut);
    return true;
}

s32 RailRider::getNextPointNo() const {
    s32 direction = -1;
    if (mIsNotReverse)
        direction = 1;
    if (this->mBezierRail->mIsClosed) {
        s32 relativePoint = mCurPoint + mBezierRail->mPointNum;
        return (s32) (relativePoint + direction + mBezierRail->mPointNum) % (s32) mBezierRail->mPointNum;
    }
    s32 nextPoint = mCurPoint + direction;
    s32 pointNum = mBezierRail->mPointNum - 1;
    if (nextPoint < 0)
        return 0;
    if (nextPoint > pointNum)
        return pointNum;
    return nextPoint;
}

/* instruction swap at the end */
void RailRider::syncPosDir() {
    if (0.0f < mCoord && mCoord < mBezierRail->getTotalLength()) {
        mBezierRail->calcPosDir(&mCurPos, &mCurDirection, mCoord);
    }
    else {
        if (mCoord == 0.0f) {
            mBezierRail->calcPos(&mCurPos, mCoord);
            mBezierRail->calcDirection(&mCurDirection, 0.1f);
        }
        else {
            mBezierRail->calcPos(&mCurPos, mCoord);
            mBezierRail->calcDirection(&mCurDirection, (mBezierRail->getTotalLength() - 0x1f));
        }
    }

    if (!mIsNotReverse) {
        mCurDirection.x *= -1.0f;
        mCurDirection.y *= -1.0f;
        mCurDirection.z *= -1.0f;
    }

    JMapInfoIter iter(nullptr, -1);
    mBezierRail->calcCurrentRailCtrlPointIter(&iter, mCoord, mIsNotReverse);
    iter.getValue<s32>("id", &mCurPoint);
}
