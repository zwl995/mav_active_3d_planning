#ifndef MAV_ACTIVE_3D_PLANNING_TRAJECTORY_EVALUATOR_H
#define MAV_ACTIVE_3D_PLANNING_TRAJECTORY_EVALUATOR_H

#include "mav_active_3d_planning/trajectory_segment.h"
#include "mav_active_3d_planning/module.h"
#include "mav_active_3d_planning/defaults.h"

#include <voxblox_ros/esdf_server.h>
#include <ros/node_handle.h>
#include <ros/console.h>
#include <Eigen/Core>

#include <string>
#include <memory>

namespace mav_active_3d_planning {

    // Forward declaration
    class CostComputer;
    class ValueComputer;
    class NextSelector;
    class EvaluatorUpdater;

    // Base class for trajectory evaluators to provide uniform interface with other classes
    class TrajectoryEvaluator : public Module {
    public:
        TrajectoryEvaluator(std::shared_ptr<voxblox::EsdfServer> voxblox_ptr, std::string param_ns);

        virtual ~TrajectoryEvaluator() {}

        // compute the gain of a TrajectorySegment
        virtual bool computeGain(TrajectorySegment *traj_in) = 0;

        // compute the cost of a TrajectorySegment
        virtual bool computeCost(TrajectorySegment *traj_in);

        // compute the Value of a segment with known cost and gain
        virtual bool computeValue(TrajectorySegment *traj_in);

        // return the index of the most promising child segment
        virtual int selectNextBest(const TrajectorySegment &traj_in);

        // Whether and how to update existing segments when a new trajectory is executed
        virtual bool updateSegments(TrajectorySegment *root);

    protected:
        friend class ModuleFactory;

        TrajectoryEvaluator() {}

        // Voxblox map
        std::shared_ptr<voxblox::EsdfServer> voxblox_ptr_;

        // bounding volume of interesting target
        defaults::BoundingVolume bounding_volume_;

        // params
        std::string p_namespace_;

        // default modules
        std::unique_ptr<CostComputer> cost_computer_;
        std::unique_ptr<ValueComputer> value_computer_;
        std::unique_ptr<NextSelector> next_selector_;
        std::unique_ptr<EvaluatorUpdater> evaluator_updater_;

        // factory accessors
        void setVoxbloxPtr(const std::shared_ptr<voxblox::EsdfServer> &voxblox_ptr);

        virtual void setupFromParamMap(Module::ParamMap *param_map);
    };

    // Abstract wrapper for default/modular implementations of the computeCost method
    class CostComputer : public Module {
    public:
        virtual bool computeCost(TrajectorySegment *traj_in) = 0;
    };

    // Abstract wrapper for default/modular implementations of the computeValue method
    class ValueComputer : public Module {
    public:
        virtual bool computeValue(TrajectorySegment *traj_in) = 0;
    };

    // Abstract wrapper for default/modular implementations of the selectNextBest method
    class NextSelector : public Module {
    public:
        virtual int selectNextBest(const TrajectorySegment &traj_in) = 0;
    };

    // Abstract wrapper for default/modular implementations of the updateSegments method
    class EvaluatorUpdater : public Module {
    public:
        EvaluatorUpdater(TrajectoryEvaluator* parent = nullptr) : parent_(parent) {};

        virtual bool updateSegments(TrajectorySegment *root) = 0;

    protected:
        friend class ModuleFactory;

        TrajectoryEvaluator* parent_;   // modules are unique ptrs, parent is always valid

        void setParent(TrajectoryEvaluator* parent);
    };

    // Utility class that finds visible voxels. Available for all trajectory generators, improve performance here.
    class RayCaster {
    public:
        RayCaster(std::shared_ptr<voxblox::EsdfServer> voxblox_ptr, std::string param_ns);

        RayCaster() {}

        virtual ~RayCaster() {}

        // Return the voxel centers of all visible voxels for a simple camera model pointing in x-direction
        bool getVisibleVoxels(std::vector <Eigen::Vector3d> *result, const Eigen::Vector3d &position,
                              const Eigen::Quaterniond &orientation);

    protected:
        // voxblox map
        std::shared_ptr<voxblox::EsdfServer> voxblox_ptr_;

        // parameters
        double p_ray_length_;
        double p_focal_length_;
        double p_ray_step_;
        int p_resolution_x_;
        int p_resolution_y_;

        // constants
        voxblox::FloatingPoint c_voxel_size_;
        voxblox::FloatingPoint c_block_size_;
        double c_field_of_view_x_;
        double c_field_of_view_y_;
    };

}  // namespace mav_active_3d_planning
#endif //MAV_ACTIVE_3D_PLANNING_TRAJECTORY_EVALUATOR_H
