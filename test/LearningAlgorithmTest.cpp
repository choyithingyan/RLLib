/*
 * Copyright 2013 Saminda Abeyruwan (saminda@cs.miami.edu)
 *
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
 *
 * LearningAlgorithmTest.cpp
 *
 *  Created on: Dec 18, 2012
 *      Author: sam
 */

#include "LearningAlgorithmTest.h"
#include "Math.h"

RLLIB_TEST_MAKE(SupervisedAlgorithmTest)

void SupervisedAlgorithmTest::linearRegressionWithTileFeatures()
{
  // simple sine curve estimation
  // training samples
  multimap<double, double> X;
  for (int i = 0; i < 100; i++)
  {
    double x = -M_PI_2 + 2 * M_PI * Probabilistic::nextDouble(); // @@>> input noise?
    double y = sin(2 * x); // @@>> output noise?
    X.insert(make_pair(x, y));
  }

  // train
  int numObservations = 1;
  int memorySize = 512;
  int numTiling = 16;
  TileCoderHashing<double> coder(memorySize, numTiling, true);
  PVector<double> x(numObservations);
  Adaline<double> adaline(coder.dimension(), 0.1 / coder.vectorNorm());
  IDBD<double> idbd(coder.dimension(), 0.001); // This value looks good
  Autostep<double> autostep(coder.dimension());
  int traininCounter = 0;
  ofstream outFileError("visualization/linearRegressionWithTileFeaturesTrainError.dat");
  while (++traininCounter < 100)
  {
    for (multimap<double, double>::const_iterator iter = X.begin(); iter != X.end(); ++iter)
    {
      x[0] = 2.0 * iter->first / M_PI; // normalized and unit generalized
      const Vector<double>* phi = coder.project(&x);
      adaline.learn(phi, iter->second);
      idbd.learn(phi, iter->second);
      autostep.learn(phi, iter->second);
    }

    // Calculate the error
    double mse[3] = { 0 };
    for (multimap<double, double>::const_iterator iterMse = X.begin(); iterMse != X.end();
        ++iterMse)
    {
      x[0] = 2.0 * iterMse->first / M_PI;
      const Vector<double>* phi = coder.project(&x);
      mse[0] += pow(iterMse->second - adaline.predict(phi), 2) / X.size();
      mse[1] += pow(iterMse->second - idbd.predict(phi), 2) / X.size();
      mse[2] += pow(iterMse->second - autostep.predict(phi), 2) / X.size();
    }
    if (outFileError.is_open())
      outFileError << mse[0] << " " << mse[1] << " " << mse[2] << endl;
  }
  outFileError.close();

  // output
  ofstream outFilePrediction("visualization/linearRegressionWithTileFeaturesPrediction.dat");
  for (multimap<double, double>::const_iterator iter = X.begin(); iter != X.end(); ++iter)
  {
    x[0] = 2.0 * iter->first / M_PI;
    const Vector<double>* phi = coder.project(&x);
    if (outFilePrediction.is_open())
      outFilePrediction << iter->first << " " << iter->second << " " << adaline.predict(phi) << " "
          << idbd.predict(phi) << " " << autostep.predict(phi) << endl;
  }
  outFilePrediction.close();
}

void SupervisedAlgorithmTest::logisticRegressionWithTileFeatures()
{
  // simple sine curve estimation
  // training samples
  multimap<double, double> X;
  for (int i = 0; i < 50; i++)
  {
    double x = Probabilistic::nextGaussian(0.25, 0.2); // @@>> input noise?
    double y = 0; // @@>> output noise?
    X.insert(make_pair(x, y));
  }

  for (int i = 50; i < 100; i++)
  {
    double x = Probabilistic::nextGaussian(0.75, 0.2); // @@>> input noise?
    double y = 1; // @@>> output noise?
    X.insert(make_pair(x, y));
  }

  // train
  int numObservations = 1;
  int memorySize = 512;
  int numTiling = 16;
  PVector<double> x(numObservations);
  TileCoderHashing<double> coder(memorySize, numTiling, true);
  SemiLinearIDBD<double> semiLinearIdbd(coder.dimension(), 0.001 / x.dimension()); // This value looks good
  int traininCounter = 0;
  ofstream outFileError("visualization/logisticRegressionWithTileFeaturesTrainError.dat");
  while (++traininCounter < 100)
  {
    for (multimap<double, double>::const_iterator iter = X.begin(); iter != X.end(); ++iter)
    {
      x[0] = 10.0 * iter->first; // normalized and unit generalized
      const Vector<double>* phi = coder.project(&x);
      semiLinearIdbd.learn(phi, iter->second);
    }

    // Calculate the error
    double crossEntropy = 0;
    for (multimap<double, double>::const_iterator iterCrossEntropy = X.begin();
        iterCrossEntropy != X.end(); ++iterCrossEntropy)
    {
      x[0] = 10.0 * iterCrossEntropy->first;
      const Vector<double>* phi = coder.project(&x);
      crossEntropy += -iterCrossEntropy->second * log(semiLinearIdbd.predict(phi))
          - (1.0 - iterCrossEntropy->second) * log(1.0 - semiLinearIdbd.predict(phi));
    }
    if (outFileError.is_open())
      outFileError << crossEntropy << endl;
  }
  outFileError.close();

  // output
  ofstream outFilePrediction("visualization/logisticRegressionWithTileFeaturesPrediction.dat");
  for (multimap<double, double>::const_iterator iter = X.begin(); iter != X.end(); ++iter)
  {
    x[0] = 10.0 * iter->first;
    const Vector<double>* phi = coder.project(&x);
    if (outFilePrediction.is_open())
      outFilePrediction << iter->first << " " << iter->second << " " << semiLinearIdbd.predict(phi)
          << endl;
  }
  outFilePrediction.close();

}

void SupervisedAlgorithmTest::linearRegressionWithRegularFeatures()
{
  // simple sine curve estimation
  // training samples
  multimap<double, double> X;
  for (int i = 0; i < 100; i++)
  {
    double x = -M_PI_2 + 2 * M_PI * Probabilistic::nextDouble(); // @@>> input noise?
    double y = sin(2 * x); // @@>> output noise?
    X.insert(make_pair(x, y));
  }

  // train
  int numObservations = 1;
  PVector<double> phi(numObservations + 1);
  phi.setEntry(phi.dimension() - 1, 1.0);
  Adaline<double> adaline(phi.dimension(), 0.00001);
  IDBD<double> idbd(phi.dimension(), 0.001); // This value looks good
  Autostep<double> autostep(phi.dimension());
  int traininCounter = 0;
  ofstream outFileError("visualization/linearRegressionWithRegularFeaturesTrainError.data");
  while (++traininCounter < 100)
  {
    for (multimap<double, double>::const_iterator iter = X.begin(); iter != X.end(); ++iter)
    {
      phi.setEntry(0, iter->first);
      adaline.learn(&phi, iter->second);
      idbd.learn(&phi, iter->second);
      autostep.learn(&phi, iter->second);
    }

    // Calculate the error
    double mse[3] = { 0 };
    for (multimap<double, double>::const_iterator iterMse = X.begin(); iterMse != X.end();
        ++iterMse)
    {
      phi.setEntry(0, iterMse->first);
      mse[0] += pow(iterMse->second - adaline.predict(&phi), 2) / X.size();
      mse[1] += pow(iterMse->second - idbd.predict(&phi), 2) / X.size();
      mse[2] += pow(iterMse->second - autostep.predict(&phi), 2) / X.size();
    }
    if (outFileError.is_open())
      outFileError << mse[0] << " " << mse[1] << " " << mse[2] << endl;
  }
  outFileError.close();

  // output
  ofstream outFilePrediction("visualization/linearRegressionWithRegularFeaturesPrediction.data");
  for (multimap<double, double>::const_iterator iter = X.begin(); iter != X.end(); ++iter)
  {
    phi.setEntry(0, iter->first);
    if (outFilePrediction.is_open())
      outFilePrediction << iter->first << " " << iter->second << " " << adaline.predict(&phi) << " "
          << idbd.predict(&phi) << " " << autostep.predict(&phi) << endl;
  }
  outFilePrediction.close();
}

void SupervisedAlgorithmTest::logisticRegressionWithRegularFeatures()
{
  // simple sine curve estimation
  // training samples
  multimap<double, double> X;
  for (int i = 0; i < 50; i++)
  {
    double x = Probabilistic::nextGaussian(0.25, 0.2); // @@>> input noise?
    double y = 0; // @@>> output noise?
    X.insert(make_pair(x, y));
  }

  for (int i = 50; i < 100; i++)
  {
    double x = Probabilistic::nextGaussian(0.75, 0.2); // @@>> input noise?
    double y = 1; // @@>> output noise?
    X.insert(make_pair(x, y));
  }

  // train
  int numObservations = 1;
  PVector<double> phi(numObservations + 1);
  phi.setEntry(phi.dimension() - 1, 1.0);
  SemiLinearIDBD<double> semiLinearIdbd(phi.dimension(), 0.001 / phi.dimension()); // This value looks good
  int traininCounter = 0;
  ofstream outFileError("visualization/logisticRegressionWithRegularFeaturesTrainError.data");
  while (++traininCounter < 100)
  {
    for (multimap<double, double>::const_iterator iter = X.begin(); iter != X.end(); ++iter)
    {
      phi.setEntry(0, iter->first);
      semiLinearIdbd.learn(&phi, iter->second);
    }

    // Calculate the error
    double crossEntropy = 0;
    for (multimap<double, double>::const_iterator iterCrossEntropy = X.begin();
        iterCrossEntropy != X.end(); ++iterCrossEntropy)
    {
      phi.setEntry(0, iterCrossEntropy->first);
      crossEntropy += -iterCrossEntropy->second * log(semiLinearIdbd.predict(&phi))
          - (1.0 - iterCrossEntropy->second) * log(1.0 - semiLinearIdbd.predict(&phi));
    }
    if (outFileError.is_open())
      outFileError << crossEntropy << endl;
  }
  outFileError.close();

  // output
  ofstream outFilePrediction("visualization/logisticRegressionWithRegularFeaturesPrediction.data");
  for (multimap<double, double>::const_iterator iter = X.begin(); iter != X.end(); ++iter)
  {
    phi.setEntry(0, iter->first);
    if (outFilePrediction.is_open())
      outFilePrediction << iter->first << " " << iter->second << " " << semiLinearIdbd.predict(&phi)
          << endl;
  }
  outFilePrediction.close();

}

void SupervisedAlgorithmTest::run()
{
  linearRegressionWithTileFeatures();
  logisticRegressionWithTileFeatures();
  linearRegressionWithRegularFeatures();
  logisticRegressionWithRegularFeatures();
}

