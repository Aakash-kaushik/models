/**
 * @file resnet.hpp
 * @author Aakash Kaushik
 * 
 * Definition of ResNet models.
 * 
 * For more information, kindly refer to the following paper.
 * 
 * Paper for ResNet.
 *
 * @code
 * @article{Kaiming He2015,
 *  author = {Kaiming He, Xiangyu Zhang, Shaoqing Ren, Jian Sun},
 *  title = {Deep Residual Learning for Image Recognition},
 *  year = {2015},
 *  url = {https://arxiv.org/pdf/1512.03385.pdf}
 * }
 * @endcode
 * 
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MODELS_MODELS_RESNET_RESNET_HPP
#define MODELS_MODELS_RESNET_RESNET_HPP

#include <mlpack/core.hpp>
#include <mlpack/methods/ann/layer/layer.hpp>
#include <mlpack/methods/ann/ffn.hpp>
#include <mlpack/methods/ann/layer/layer_types.hpp>
#include <mlpack/methods/ann/init_rules/random_init.hpp>
#include <mlpack/methods/ann/loss_functions/binary_cross_entropy_loss.hpp>
#include <mlpack/methods/ann/init_rules/he_init.hpp>

namespace mlpack {
namespace models {

template<
  typename OutputLayerType = ann::CrossEntropyError<>,
  typename InitializationRuleType = ann::RandomInitialization,
  size_t ResNetVersion = 18
>
class ResNet{
 public:

  ResNet();

  ResNet(const bool includeTop = true,
         const bool preTrained = false,
         const size_t inputChannel,
         const size_t inputWidth,
         const size_t inputHeight,
         const size_t numClasses = 1000);

  ResNet(const bool includeTop = true,
         const bool preTrained = false,
         std::tuple<size_t, size_t, size_t> inputShape,
         const size_t numClasses = 1000);

  ann::FFN<OutputLayerType, InitializationRuleType> GetModel()
      { return resNet; }

  void LoadModel(const std::string& filePath);

  void SaveModel(const std::string& filepath);

 private:

  template<typename SequentialType = ann::Sequential<>>
  void ConvolutionBlock3x3(SequentialType* baseLayer,
                           const size_t inSize,
                           const size_t outSize,
                           const size_t kernelWidth = 3,
                           const size_t kernelHeight = 3,
                           const size_t strideWidth = 1,
                           const size_t strideHeight = 1,
                           const size_t padW = 1,
                           const size_t padH = 1)
  {
    baseLayer->Add(ann::Convolution<>(inSize, outSize, kernelWidth,
        kernelHeight, strideWidth, strideHeight, padW, padH, inputWidth,
        inputHeight));

    // Updating input dimesntions.
    inputwidth = ConvOutSize(inputWidth, kernelWidth, strideWidth, padW);
    inputHeight = ConvOutSize(inputHeight, kernelHeight, strideHeight, padH);
  }

  template<typename SequentialType = ann::Sequential<>>
  void ConvolutionBlock1x1(SequentialType* baseLayer,
                           const size_t inSize,
                           const size_t outSize,
                           const size_t kernelWidth = 1,
                           const size_t kernelHeight = 1,
                           const size_t strideWidth = 1,
                           const size_t strideHeight = 1,
                           const size_t padW = 0,
                           const size_t padH = 0)
  {
    baseLayer->Add(ann::Convolution<>(inSize, outSize, kernelWidth,
        kernelHeight, strideWidth, strideHeight, padW, padH, inputWidth,
        inputHeight));

    // Updating input dimesntions.
    inputwidth = ConvOutSize(inputWidth, kernelWidth, strideWidth, padW);
    inputHeight = ConvOutSize(inputHeight, kernelHeight, strideHeight, padH);
  }

  template <typename AddmergeType = ann::AddMerge<>>
  void DownSample(AddmergeType* downSample,
                  const size_t inSize,
                  const size_t outSize,
                  const size_t kernelWidth = 1,
                  const size_t kernelHeight = 1,
                  const size_t strideWidth = 1,
                  const size_t strideHeight = 1,
                  const size_t padW = 0,
                  const size_t padH = 0)
  {
    ConvolutionBlock1x1(downSample, inSize, outSize, kernelWidth, kernelHeight,
        strideWidth, strideHeight, padW, padH, inputWidth, inputHeight);
    downSample->Add(BatchNorm<>(outSize));
  }

  void BasicBlock(const size_t inSize,
                  const size_t outSize,
                  const bool downSample = false;
                  const size_t kernelWidth = 1,
                  const size_t kernelHeight = 1)
  {  
    ann::Sequential<>* basicBlock = new Sequential<>();
    ann::AddMerge<>* resBlock = new ann::AddMerge<>(true, true);
    ann::Sequential<>* sequentialBlock = new ann::Sequential<>();
    ConvolutionBlock3x3(seqentialBlock, inSize, outSize, kernelWidth,
        kernelHeight)
    seqentialBlock->Add(BatchNorm<>(outSize));
    seqentialBlock->Add(ann::ReLULayer<>);
    ConvolutionBlock3x3(seqentialBlock, outSize, outSize);
    seqentialBlock->Add(BatchNorm<>(outSize));

    resBlock->Add(seqentialBlock);

    if (downSample == true)
      DownSample(resBlock, inSize, outSize);
    else
      resBlock->Add(IdentityLayer<>)

    basicBlock->Add(resBlock);
    basicBlock->Add(ann::ReLULayer<>);
    resNet.Add(basicBlock);
  }

  void BottleNeck()
  {
  }

  void MakeLayer(const std::string& block,
                 const size_t outSize,
                 const size_t numBlocks,
                 const size_t stride = 1)
  {
    size_t inSize = 64;
    bool downSample = false;

    if (block == "basicblock")
    {
      if (stride != 1 || inSize != inSize * basicBlockExpansion)
        downSample = true;
      basicBlock(inSize, outSize, downSample);
      inSize = inSize * basicBlockExpansion;
      for (size_t i = 1; i != numBlocks; ++i)
        basicBlock(inSize, outSize)
    }

    else if (block == "bottleneck")
    {
      if (stride != 1 || inSize != inSize * bottleNeckExpansion)
        downSample = true;
      bottleneck(inSize, outSize, downSample);
      inSize = inSize * bottleNeckExpansion;
      for (size_t i = 1; i != numBlocks; ++i)
        bottleneck(inSize, outSize)
    }
  }
  size_t ConvOutSize(const size_t size,
                     const size_t k,
                     const size_t s,
                     const size_t padding)
  {
    return std::floor(size + 2 * padding - k) / s + 1;
  }

  ann::FFN<OutputLayerType, InitializationRuleType> resNet;
  size_t inputChannel;
  size_t inputWidth;
  size_t inputHeight;
  size_t numClasses;
  size_t basicBlockExpansion = 1;
  size_t bottleNeckExpansion = 4;
}; // ResNet class

} // namespace models
} // namespace mlpack

#include "resnet_impl.hpp"

#endif
