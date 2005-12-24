#ifndef SIMTK_MASS_ELEMENT_REP_H_
#define SIMTK_MASS_ELEMENT_REP_H_

/* Copyright (c) 2005-6 Stanford University and Michael Sherman.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including 
 * without limitation the rights to use, copy, modify, merge, publish, 
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**@file
 * Declarations for the *real* MassElement objects. These are opaque to
 * users.
 */

#include "MassElement.h"
#include "FeatureRep.h"

namespace simtk {

/**
 * This is a still-abstract Subsystem representation, common to all
 * the MassElement features.
 */
class MassElementRep : public FeatureRep {
public:
    MassElementRep(MassElement& m, const std::string& nm, const Placement& sample) 
        : FeatureRep(m,nm,sample) { }
    // must call initializeStandardSubfeatures() to complete construction.

    const RealMeasure& getMassMeasure() const
      { return RealMeasure::downcast(getFeature(massMeasureIndex)); }
    RealMeasure&       updMassMeasure()
      { return RealMeasure::downcast(updFeature(massMeasureIndex)); }

    const StationMeasure& getCentroidMeasure() const
      { return StationMeasure::downcast(getFeature(centroidMeasureIndex)); }
    StationMeasure&       updCentroidMeasure()
      { return StationMeasure::downcast(updFeature(centroidMeasureIndex)); }

    const InertiaMeasure& getInertiaMeasure() const
      { return InertiaMeasure::downcast(getFeature(inertiaMeasureIndex)); }
    InertiaMeasure&       updInertiaMeasure()
      { return InertiaMeasure::downcast(updFeature(inertiaMeasureIndex)); }

    // virtuals getFeatureTypeName() && clone() still missing

    SIMTK_DOWNCAST(MassElementRep,SubsystemRep);

protected:
    // Every MassElement defines some mass-oriented measures.
    virtual void initializeStandardSubfeatures() {
        Feature& mm = addFeatureLike(RealMeasure("massMeasure"), "massMeasure");
        Feature& cm = addFeatureLike(StationMeasure("centroidMeasure"), "centroidMeasure");
        Feature& im = addFeatureLike(InertiaMeasure("inertiaMeasure"), "inertiaMeasure");
        massMeasureIndex     = mm.getIndexInParent();
        centroidMeasureIndex = cm.getIndexInParent();
        inertiaMeasureIndex  = im.getIndexInParent();
    }

private:
    int massMeasureIndex, centroidMeasureIndex, inertiaMeasureIndex;
};

class PointMassElementRep : public MassElementRep {
public:
    PointMassElementRep(PointMassElement& pm, const std::string& nm) 
        : MassElementRep(pm,nm,StationPlacement(Vec3(NTraits<Real>::getNaN()))), 
          massIndex(-1) { }
    // must call initializeStandardSubfeatures() to complete construction.

    void setPointMass(const Real& m)
      { updPointMass().place(RealPlacement(m)); }
    const RealParameter& getPointMass() const
      { return RealParameter::downcast(getFeature(massIndex)); }
    RealParameter& updPointMass()
      { return RealParameter::downcast(updFeature(massIndex)); }

    std::string getFeatureTypeName() const { return "PointMassElement"; }
    SubsystemRep* clone() const { return new PointMassElementRep(*this); }

    PlacementRep* createFeatureReference(Placement& p, int i) const { 
        PlacementRep* prep=0;
        if (i == -1) 
            prep = new StationFeaturePlacementRep(getMyHandle());
        else if (0<=i && i<3)
            prep = new RealFeaturePlacementRep(getMyHandle(), i);
        if (prep) {
            prep->setMyHandle(p); p.setRep(prep);
            return prep;
        }

        SIMTK_THROW3(Exception::IndexOutOfRangeForFeaturePlacementReference,
            getFullName(), getFeatureTypeName(), i);
        //NOTREACHED
        return 0;
    }

    SIMTK_DOWNCAST(PointMassElementRep,SubsystemRep);

protected:
    // This will be called from initializeStandardSubfeatures() in MassElement.
    virtual void initializeStandardSubfeatures() {
        MassElementRep::initializeStandardSubfeatures();

        massIndex = addFeatureLike(RealParameter("mass"), "mass").getIndexInParent();

        updMassMeasure().place(getPointMass());
        updCentroidMeasure().place(getMyHandle());
        updInertiaMeasure().place(InertiaPlacement(getCentroidMeasure(), getMassMeasure()));
    }

    int massIndex;
};

class CylinderMassElementRep : public MassElementRep {
public:
    CylinderMassElementRep(CylinderMassElement& cm, const std::string& nm) 
      : MassElementRep(cm,nm,Placement()),  // "void" placement
        massIndex(-1), radiusIndex(-1), halfLengthIndex(-1), centerIndex(-1), axisIndex(-1)
    { }
    // must call initializeStandardSubfeatures() to complete construction.

    // some self-placements
    void setMass(const Real& m) {
        updFeature(massIndex).place(RealPlacement(m));
    }
    void setRadius(const Real& r) {
        updFeature(radiusIndex).place(RealPlacement(r));
    }
    void setHalfLength(const Real& h) {
        updFeature(halfLengthIndex).place(RealPlacement(h));
    }
    void placeCenter(const Vec3& c) {
        updFeature(centerIndex).place(StationPlacement(c));
    }
    void placeAxis(const Vec3& a) {
        updFeature(axisIndex).place(DirectionPlacement(a));
    }

    std::string getFeatureTypeName() const { return "CylinderMassElement"; }

    FeatureRep* clone() const { return new CylinderMassElementRep(*this); }
    PlacementRep* createFeatureReference(Placement&, int) const {
        SIMTK_THROW2(Exception::NoFeatureLevelPlacementForThisKindOfFeature,
            getFullName(), getFeatureTypeName());
        //NOTREACHED
        return 0;
    }

    SIMTK_DOWNCAST(CylinderMassElementRep,SubsystemRep);
private:
    // This will be called from initializeStandardSubfeatures() in MassElement.
    virtual void initializeStandardSubfeatures() {
        MassElementRep::initializeStandardSubfeatures();

        massIndex       = addFeatureLike(RealParameter("mass"),       "mass")
                            .getIndexInParent();
        radiusIndex     = addFeatureLike(RealParameter("radius"),     "radius")
                            .getIndexInParent();
        halfLengthIndex = addFeatureLike(RealParameter("halfLength"), "halfLength")
                            .getIndexInParent();
        centerIndex     = addFeatureLike(Station      ("center"),     "center")
                            .getIndexInParent();
        axisIndex       = addFeatureLike(Direction    ("axis"),       "axis")
                            .getIndexInParent();

        updMassMeasure().place(getFeature(massIndex));
        updCentroidMeasure().place(getFeature(centerIndex));

        // MatInertia about the body origin: get the cylinder's principal moments in its
        // own frame (with z being the long axis, and the origin at the COM). Then
        // transform to the body frame and shift to the body origin.
        const Real oo3 = Real(1)/Real(3);   // "one over" 3
        const RealPlacement      r(getFeature(radiusIndex));
        const RealPlacement      h(getFeature(halfLengthIndex));
        const RealPlacement      m(getFeature(massIndex));
        const DirectionPlacement z(getFeature(axisIndex));
        const StationPlacement   c(getFeature(centerIndex));
        const RealPlacement      Izz(m*0.5*square(r));
        const RealPlacement      Ixx(m*(0.25*square(r) + oo3*square(h)));

        // This is a reference frame for the cylinder, measured from and expressed
        // in the body frame. The x and y axes are arbitrary due to symmetry.
        const FramePlacement     F_BC(OrientationPlacement(z,2), c);

        updInertiaMeasure().place(
            InertiaPlacement(Ixx,Ixx,Izz).xformFromCOM(~F_BC));
    }

    int massIndex, radiusIndex, halfLengthIndex, centerIndex, axisIndex;
};

} // namespace simtk


#endif // SIMTK_MASS_ELEMENT_REP_H_
