/*
 * Copyright (c) Glow Contributors. See CONTRIBUTORS file.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GLOW_NNPI_BACKEND_H
#define GLOW_NNPI_BACKEND_H

#include "NNPIAdapterContainer.h"
#include "NNPIOptions.h"
#include "glow/Backend/Backend.h"

namespace glow {

/// This is the Intel Neural-Network Processor for Inference (NNPI) backend.
class NNPIBackend final : public Backend {
public:
  /// Ctor.
  explicit NNPIBackend() = default;

  /// @name Backend methods.
  /// This is the implementation of the Backend interface.
  ///@{
  ~NNPIBackend() override = default;

  std::string getBackendName() const override { return getName(); }
  static std::string getName() { return "NNPI"; }
  static unsigned numDevices();

  Expected<std::unique_ptr<CompiledFunction>>
  compile(Function *F, const BackendOptions &opts) const override;

  bool acceptForExecution(const NodeInfo &NI) const override;
  bool isOpSupported(const NodeInfo &NI) const override;
  bool shouldLower(const Node *N) const override;
  bool shouldShareBuffers() const override { return false; }
  bool supportsPartialTensors() const override { return true; }
  bool supportsStaticPlaceholders() const override { return true; }
  std::unique_ptr<FunctionPassPipeline>
  getOptimizationPipeline() const override;

  runtime::DeviceManager *
  createDeviceManager(const runtime::DeviceConfig &deviceConfig) override;

  Expected<bool> transformPostLowering(
      Function *F, CompilationContext &cctx,
      const glow::runtime::DeviceInfo *devInfo = nullptr) const override;

  virtual llvm::StringMap<std::string>
  getSupportedCompiledFunctionOptions() const override {
    NNPICompilationOptions options({});
    return options.getSupportedOptions();
  };

  virtual llvm::StringMap<std::string>
  getSupportedDeviceManagerOptions() const override {
    NNPIDeviceOptions options({});
    return options.getSupportedOptions();
  };

  virtual Error bindContexts(llvm::ArrayRef<runtime::ContextBinding> bindings,
                             const runtime::DAGNode *root, bool enableP2P,
                             bool enableDRT) override;

  /// Estimate performance cost for a given Node \p N.
  /// Returns a unitless value to be used when comparing to other estimates
  /// or -1 if no estimate could be generated.
  /// SparseLength and EmbeddingBag type nodes are supported.
  double
  estimateEmbeddingNode(const glow::NodeInfo &NI, bool fp32Accumulation = false,
                        glow::LengthsMode lengthsMode = LengthsMode::Variable,
                        float averageLength = NAN) const;
  /// @}

private:
#if FACEBOOK_INTERNAL
  /// Performs FB-private transformations on \p F given \p cctx.
  /// \returns whether \p F is modified.
  bool transformPrivate(Function *F, CompilationContext &cctx) const;
#endif /* FACEBOOK_INTERNAL */

  static NNPIBackendOptions backendOptions_;
  static NNPIAdapterContainer adapter_;
};

/// These are used for parsing backend-specific node options.
constexpr char numParallelChunksKey[] = "NNPI_numParallelChunks";
constexpr char parallelTransformKindKey[] = "NNPI_parallelTransformKind";
constexpr char extraEdgesKey[] = "NNPI_extraEdges";
constexpr char coreAssignmentsKey[] = "NNPI_coreAssignments";

/// These are used for parsing edge strings in the form of [#$]name[@#]
struct ExtraEdgeSplitPair {
  bool hasSplit;
  std::string label;
  int splitNum;
};
Expected<ExtraEdgeSplitPair>
getExtraEdgeTargetSplitPair(const std::string &edge);
Expected<ExtraEdgeSplitPair>
getExtraEdgeSourceSplitPair(const std::string &edge);

} // namespace glow
#endif // GLOW_NNPI_BACKEND_H
