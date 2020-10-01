pipeline
{
    agent none
    stages
    {
        stage ('Ubuntu 20.04')
        {
            agent {label 'Ubuntu_20.04.1'}
            stages 
            {
                stage('Checkout')
                {
                    steps
                    {
                        //checkout([$class: 'GitSCM', branches: [[name: '*/master']], doGenerateSubmoduleConfigurations: false, extensions: [], submoduleCfg: [], userRemoteConfigs: [[credentialsId: 'b27e7477-aab1-4f4e-a05c-23cd05d217ee', url: 'https://github.com/fpga-open-speech-tools/component_library.git']]])
                        sh 'git clone https://github.com/fpga-open-speech-tools/linux-socfpga.git;'
                    }
                }
                stage('Configure Linux Kernel')
                {
                    steps
                    {   dir("linux-socfpga")
                        {
                            sh 'make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- openspeech_defconfig;'
                            sh 'make prepare ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-;'
                        }
                    }
                }
                stage('AD1939 LKM')
                {
                    steps
                    {   dir("ad1939")
                        {
                            sh 'make;'
                            archiveArtifacts artifacts: '*.ko', fingerprint: true 
                        }
                    }
                }
            }
        }
    }
}