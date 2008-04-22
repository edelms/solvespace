
#ifndef __SKETCH_H
#define __SKETCH_H

class hGroup;
class hRequest;
class hEntity;
class hParam;

class Entity;
class Param;

class hEquation;
class Equation;

// All of the hWhatever handles are a 32-bit ID, that is used to represent
// some data structure in the sketch.
class hGroup {
public:
    // bits 15: 0   -- group index
    DWORD v;
};
class hRequest {
public:
    // bits 15: 0   -- request index
    DWORD   v;

    inline hEntity entity(int i);
    inline hParam param(int i);

    inline bool IsFromReferences(void);
};
class hEntity {
public:
    // bits 15: 0   -- entity index
    //      31:16   -- request index
    DWORD   v;

    inline hRequest request(void);
};
class hParam {
public:
    // bits 15: 0   -- param index
    //      31:16   -- request index
    DWORD       v;

    inline hRequest request(void);
};

// A set of requests. Every request must have an associated group.
class Group {
public:
    static const hGroup     HGROUP_REFERENCES;

    int         tag;
    hGroup      h;

    int         solveOrder;
    bool        solved;

    NameStr     name;

    char *DescriptionString(void);
};


class EntityId {
    DWORD v;        // entity ID, starting from 0
};
class EntityMap {
    int         tag;

    EntityId    h;
    hEntity     input;
    int         copyNumber;
    // (input, copyNumber) gets mapped to ((Request)xxx).entity(h.v)
};

// A user request for some primitive or derived operation; for example a
// line, or a step and repeat.
class Request {
public:
    // Some predefined requests, that are present in every sketch.
    static const hRequest   HREQUEST_REFERENCE_XY;
    static const hRequest   HREQUEST_REFERENCE_YZ;
    static const hRequest   HREQUEST_REFERENCE_ZX;

    int         tag;
    hRequest    h;

    // Types of requests
    static const int CSYS_2D                = 100;
    static const int DATUM_POINT            = 101;
    static const int LINE_SEGMENT           = 200;

    int         type;

    hEntity     csys; // or Entity::NO_CSYS
    hGroup      group;
    NameStr     name;
    bool        construction;

    // When a request generates entities from entities, and the source
    // entities may have come from multiple requests, it's necessary to
    // remap the entity ID so that it's still unique. We do this with a
    // mapping list.
    IdList<EntityId,EntityMap> remap;
    hEntity Remap(hEntity in, int copyNumber);

    hParam AddParam(IdList<Param,hParam> *param, hParam hp);
    void Generate(IdList<Entity,hEntity> *entity, IdList<Param,hParam> *param);

    char *DescriptionString(void);
};

class Entity {
public:
    int         tag;
    hEntity     h;

    static const hEntity    NO_CSYS;

    static const int CSYS_2D                = 1000;
    static const int POINT_IN_3D            = 2000;
    static const int POINT_IN_2D            = 2001;
    static const int LINE_SEGMENT           = 3000;
    int         type;

    bool        symbolic;
    // The params are usually handles to the symbolic variables, but may
    // also be constants
    union {
        hParam      h[16];
        double      v[16];
    }           param;
    // Associated entities, e.g. the endpoints for a line segment
    hEntity     assoc[16];

    hEntity     csys;   // or Entity::NO_CSYS

    // Applies only for a CSYS_2D type
    void Csys2dGetBasisVectors(Vector *u, Vector *v);
    void Csys2dGetBasisExprs(ExprVector *u, ExprVector *v);

    bool IsPoint(void);
    bool IsPointIn3d(void);
    // Applies for any of the point types
    Vector PointGetCoords(void);
    ExprVector PointGetExprs(void);
    void PointForceTo(Vector v);
    bool PointIsFromReferences(void);
    bool PointIsKnown(void);

    // Applies for anything that comes with a plane
    bool HasPlane(void);
    // The plane is points P such that P dot (xn, yn, zn) - d = 0
    void PlaneGetExprs(ExprVector *n, Expr **d);

    // Routines to draw and hit-test the representation of the entity
    // on-screen.
    struct {
        bool    drawing;
        Point2d mp;
        double  dmin;
    } dogd;
    void LineDrawOrGetDistance(Vector a, Vector b);
    void DrawOrGetDistance(int order);
    void Draw(int order);
    double GetDistance(Point2d mp);

    char *DescriptionString(void);
};

class Param {
public:
    int         tag;
    hParam      h;

    double      val;
    bool        known;
};


inline bool hRequest::IsFromReferences(void) {
    if(v == Request::HREQUEST_REFERENCE_XY.v) return true;
    if(v == Request::HREQUEST_REFERENCE_YZ.v) return true;
    if(v == Request::HREQUEST_REFERENCE_ZX.v) return true;
    return false;
}
inline hEntity hRequest::entity(int i)
    { hEntity r; r.v = (v << 16) | i; return r; }
inline hParam hRequest::param(int i)
    { hParam r; r.v = (v << 16) | i; return r; }

inline hRequest hEntity::request(void)
    { hRequest r; r.v = (v >> 16); return r; }

inline hRequest hParam::request(void)
    { hRequest r; r.v = (v >> 16); return r; }


class hConstraint {
public:
    DWORD   v;

    hEquation equation(int i);
};

class Constraint {
public:
    static const int USER_EQUATION      = 10;
    static const int POINTS_COINCIDENT  = 20;
    static const int PT_PT_DISTANCE     = 30;
    static const int PT_LINE_DISTANCE   = 31;
    static const int PT_IN_PLANE        = 40;
    static const int EQUAL_LENGTH_LINES = 50;

    static const int HORIZONTAL         = 80;
    static const int VERTICAL           = 81;

    int         tag;
    hConstraint h;

    int         type;
    hGroup      group;

    // These are the parameters for the constraint.
    Expr        *exprA;
    Expr        *exprB;
    hEntity     ptA;
    hEntity     ptB;
    hEntity     ptC;
    hEntity     entityA;
    hEntity     entityB;

    // These define how the constraint is drawn on-screen.
    struct {
        Vector      offset;
    } disp;

    static hConstraint AddConstraint(Constraint *c);
    static void MenuConstrain(int id);
    
    struct {
        bool    drawing;
        Point2d mp;
        double  dmin;
    } dogd; // state for drawing or getting distance (for hit testing)
    void LineDrawOrGetDistance(Vector a, Vector b);
    void DrawOrGetDistance(Vector *labelPos);

    double GetDistance(Point2d mp);
    Vector GetLabelPos(void);
    void Draw(void);

    bool HasLabel(void);

    void Generate(IdList<Equation,hEquation> *l);
    // Some helpers when generating symbolic constraint equations
    void ModifyToSatisfy(void);
    void AddEq(IdList<Equation,hEquation> *l, Expr *expr, int index);
    static Expr *Distance(hEntity pa, hEntity pb);
};

class hEquation {
public:
    DWORD v;
};

class Equation {
public:
    int         tag;
    hEquation   h;

    Expr        *e;
};

inline hEquation hConstraint::equation(int i)
    { hEquation r; r.v = (v << 16) | i; return r; }


#endif
