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
                stage('Check Component Library Repo') 
                {
                    parallel
                    {
                        stage('Frost Edge')
                        {
                            when {
                                anyOf {
                                    changeset "dts/**/*"
                                }
                            }
                            steps 
                            {
                                build job: 'Frost Build Base Device Tree'
                            }
                        }
                    }
                }
                stage('Cleanup')
                {
                    steps
                    {
                        deleteDir()
                        dir("${workspace}@tmp") {
                            deleteDir()
                        }
                    }
                } 
            }
        }
    }
}