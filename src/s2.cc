#include <node.h>
#include <nan.h>
#include "node_object_wrap.h"           // for ObjectWrap
#include "v8.h"                         // for Handle, String, Integer, etc

#include "s2latlng.h"
#include "s2cellid.h"
#include "s2cell.h"
#include "s2polygon.h"
#include "s2loop.h"
#include "s2polygonbuilder.h"
#include "s2regioncoverer.h"
#include "latlng.h"
#include "latlngrect.h"
#include "cell.h"
#include "cellid.h"
#include "interval.h"
#include "point.h"
#include "cap.h"
#include "regioncoverer.h"
#include "angle.h"

using namespace v8;

NAN_METHOD(GetCover) {
    NanScope();

    scoped_ptr<S2PolygonBuilder> builder(
            new S2PolygonBuilder(S2PolygonBuilderOptions::DIRECTED_XOR()));

    if (args.Length() < 1) { return NanThrowError("(array) required"); }

    if (args[0]->IsArray()) {
        Handle<Array> array = Handle<Array>::Cast(args[0]);
        for (uint32_t i = 0; i < array->Length(); i++) {
            Local<Object> obj = array->Get(i)->ToObject();
            if (NanHasInstance(LatLng::constructor, obj)) {
                LatLng *a = node::ObjectWrap::Unwrap<LatLng>(array->Get(i)->ToObject());
                LatLng *b = node::ObjectWrap::Unwrap<LatLng>(array->Get((i + 1) % array->Length())->ToObject());
                builder->AddEdge(a->get().ToPoint(), b->get().ToPoint());
            }
        }
        S2Polygon polygon;
        typedef vector<pair<S2Point, S2Point> > EdgeList;
        EdgeList edgeList;
        builder->AssemblePolygon(&polygon, &edgeList);
        S2RegionCoverer coverer;
        std::vector<S2CellId> cellids_vector;
        coverer.GetCovering(polygon, &cellids_vector);
        Local<Array> out = Array::New(cellids_vector.size());
        for (int i = 0; i < cellids_vector.size(); i++) {
            out->Set(i, Cell::New(cellids_vector.at(i)));
        }
        NanReturnValue(out);
    }

    NanReturnValue(NanNew<String>("hi"));
}

void RegisterModule(Handle<Object> exports) {
    LatLng::Init(exports);
    LatLngRect::Init(exports);
    Cap::Init(exports);
    Angle::Init(exports);
    Cell::Init(exports);
    // RegionCoverer::Init(exports);
    CellId::Init(exports);
    Point::Init(exports);
    Interval::Init(exports);
    exports->Set(NanSymbol("_getCover"),
            NanNew<FunctionTemplate>(GetCover)->GetFunction());
}

NODE_MODULE(_s2, RegisterModule);