#ifndef slic3r_SLAPrint_hpp_
#define slic3r_SLAPrint_hpp_

#include "PrintBase.hpp"
#include "Point.hpp"
//#include "SLA/SLASupportTree.hpp"

namespace Slic3r {

enum SLAPrintStep {
	slapsRasterize,
	slapsValidate,
	slapsCount
};

enum SLAPrintObjectStep {
	slaposObjectSlice,
	slaposSupportIslands,
	slaposSupportPoints,
	slaposSupportTree,
	slaposBasePool,
	slaposSliceSupports,
	slaposCount
};

class SLAPrint;

class SLAPrintObject : public PrintObjectBaseWithState<SLAPrint, SLAPrintObjectStep, slaposCount>
{
private: // Prevents erroneous use by other classes.
    typedef PrintObjectBaseWithState<Print, SLAPrintObjectStep, slaposCount> Inherited;

public:
    const ModelObject*      model_object() const    { return m_model_object; }
    ModelObject*            model_object()          { return m_model_object; }

protected:
    // to be called from SLAPrint only.
    friend class SLAPrint;

	SLAPrintObject(SLAPrint* print, ModelObject* model_object);
	~SLAPrintObject() {}

    void                    config_apply(const ConfigBase &other, bool ignore_nonexistent = false) { this->m_config.apply(other, ignore_nonexistent); }
    void                    config_apply_only(const ConfigBase &other, const t_config_option_keys &keys, bool ignore_nonexistent = false) 
    	{ this->m_config.apply_only(other, keys, ignore_nonexistent); }
    void                    set_trafo(const Transform3d& trafo) { m_trafo = trafo; }

    struct Instance {
	    // Slic3r::Point objects in scaled G-code coordinates
    	Point 	shift;
    	// Rotation along the Z axis, in radians.
    	float 	rotation; 
    };
    bool                    set_instances(const std::vector<Instance> &instances);
    // Invalidates the step, and its depending steps in SLAPrintObject and SLAPrint.
    bool                    invalidate_step(SLAPrintObjectStep step);

private:
	// Points to the instance owned by a Model stored at the parent SLAPrint instance.
    ModelObject                            *m_model_object;
    // Object specific configuration, pulled from the configuration layer.
    SLAPrintObjectConfig                    m_config;
    // Translation in Z + Rotation by Y and Z + Scaling / Mirroring.
    Transform3d                             m_trafo = Transform3d::Identity();
    std::vector<Instance> 					m_instances;

//    sla::EigenMesh3D emesh;
//    std::unique_ptr<sla::SLASupportTree> support_tree_ptr;
//    SlicedSupports slice_cache;

	friend SLAPrint;
};

/**
 * @brief This class is the high level FSM for the SLA printing process.
 *
 * It should support the background processing framework and contain the
 * metadata for the support geometries and their slicing. It should also
 * dispatch the SLA printing configuration values to the appropriate calculation
 * steps.
 *
 * TODO (decide): The last important feature is the support for visualization
 * which (at least for now) will be implemented as a method(s) returning the
 * triangle meshes or receiving the rendering canvas and drawing on that
 * directly.
 *
 * TODO: This class uses the BackgroundProcess interface to create workers and
 * manage input change events. An appropriate implementation can be derived
 * from BackgroundSlicingProcess which is now working only with the FDM Print.
 */
class SLAPrint : public PrintBaseWithState<SLAPrintStep, slapsCount>
{
private: // Prevents erroneous use by other classes.
    typedef PrintBaseWithState<SLAPrintStep, slapsCount> Inherited;

public:
    SLAPrint() {}
	virtual ~SLAPrint() { this->clear(); }

	PrinterTechnology	technology() const noexcept { return ptSLA; }

    void                clear() override;
    bool                empty() const override { return false; }
    ApplyStatus         apply(const Model &model, const DynamicPrintConfig &config) override;
    void                process() override;

private:
    Model                           m_model;
    SLAPrinterConfig                m_printer_config;
    SLAMaterialConfig               m_material_config;

	std::vector<SLAPrintObject*>	m_objects;

	friend SLAPrintObject;
};

} // namespace Slic3r

#endif /* slic3r_SLAPrint_hpp_ */