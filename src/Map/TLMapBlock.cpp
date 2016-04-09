#include "Map/TLMapBlock.h"
#include "MC/AssetsManager.h"
#include "MC/MCLoader.h"
#include "Common/TLModel.h"
#include "map.pb.h"

#define GRID_SPRITE_FILE "images/GridLine.png"

TLMapBlock::TLMapBlock( const std::string& strMapBlockFile )
{
    m_pMaterialBatchNode = NULL;

    m_nRow = 0;
    m_nCol = 0;
	m_nWidth = 0;
	m_nHeight = 0;

    m_bShowGridLine = true;

	CCTexture2D* pGridTexture = MCLoader::sharedMCLoader()->loadTexture( GRID_SPRITE_FILE );
    m_pGridBatchNode = CCSpriteBatchNode::createWithTexture( pGridTexture );
    addChild( m_pGridBatchNode, 10000 );

#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
    m_strMapBlockFile = strMapBlockFile;

	m_pSelectedNode = NULL;
	m_pSelMarkSprite = MCLoader::sharedMCLoader()->loadSprite( "images/selected.png" );
    m_pSelMarkSprite->setVisible( false );
	m_kSelMarkSize = m_pSelMarkSprite->getContentSize();
	addChild( m_pSelMarkSprite, 1024 );

#else
    if( strMapBlockFile.find( "./" ) != std::string::npos )
    {
        m_strMapBlockFile = strMapBlockFile.substr( 2 );
    }
    else
    {
        m_strMapBlockFile = strMapBlockFile;
    }
#endif
}

TLMapBlock::~TLMapBlock()
{
    clear();
}

TLMapBlock* TLMapBlock::create( const std::string& strFileName )
{
    TLMapBlock * pRet = new TLMapBlock( strFileName );
    if( pRet && pRet->init() )
    {
        pRet->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(pRet);
    }
    return pRet;
}

#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
bool TLMapBlock::newMapBlock( const std::string& strMapBlockFile, const std::string& strBlockName, int nRow, int nCol, int nGridWidth, int nGridHeight, const std::string& strMaterial )
{
    FILE* fp = fopen( strMapBlockFile.c_str(), "wb" );
    if( fp == NULL )
        return false;

    framework::MapBlock mbData;
	mbData.set_name( strBlockName );
    mbData.set_row( nRow );
    mbData.set_col( nCol );
    mbData.set_width( nGridWidth );
    mbData.set_height( nGridHeight );
    mbData.set_material( strMaterial );
    
    int nGridState = TL_GRID_FLAG_PLANT | TL_GRID_FLAG_PASS;
    for( int i=0; i < nRow * nCol; ++i )
        mbData.add_grid_states( nGridState );

    std::string strBuffer;
    mbData.SerializeToString( &strBuffer );

    fwrite( strBuffer.c_str(), strBuffer.length(), 1, fp );

    fclose( fp );

    return true;
}
#endif

bool TLMapBlock::init()
{
    if( !CCNode::init() )
        return false;

	unsigned long iSize = 0;
	unsigned char* pBuffer = AssetsManager::sharedAssetsManager()->getFileContent( m_strMapBlockFile.c_str(), "rb", &iSize );
    if( pBuffer == NULL )
        return false;

    // 先清理
    clear();

    framework::MapBlock mbData;
	mbData.ParseFromArray( (void*)pBuffer, iSize );

	delete[] pBuffer;

    // 
	m_strBlockName = mbData.name();
    m_nRow = mbData.row();
    m_nCol = mbData.col();
    m_nWidth = mbData.width();
    m_nHeight = mbData.height();
#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
    m_strMaterial = mbData.material();
#endif

    // 地表材质
    std::string strMaterial = mbData.material();
    updateMaterial( strMaterial, mbData.col() * mbData.width(), mbData.row() * mbData.height() );

    // 地表网格的状态
    m_vecGridStates.resize( m_nRow * m_nCol );
    for( int i=0; i < mbData.grid_states_size(); ++i )
        m_vecGridStates[i] = mbData.grid_states( i );

    // 地表装饰物
    for( int i=0; i < mbData.sprites_size(); ++i )
    {
        const framework::SpriteInfo& si = mbData.sprites( i );

        CCSprite* pSprite = MCLoader::sharedMCLoader()->loadSprite( si.file().c_str() );
        pSprite->setPositionX( si.x() );
        pSprite->setPositionY( si.y() );
        pSprite->setScale( si.scale() );
        pSprite->setRotation( si.rotation() );
        addChild( pSprite, si.z_order() );

#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
        SpriteInfo* pSpriteInfo = new SpriteInfo;
        pSpriteInfo->pSprite = pSprite;
        pSpriteInfo->strFileName = si.file();
        pSpriteInfo->x = si.x();
        pSpriteInfo->y = si.y();
        pSpriteInfo->scale = si.scale();
        pSpriteInfo->rotation = si.rotation();
        pSpriteInfo->z_order = si.z_order();

        m_listAllSprites.push_back( pSpriteInfo );
#endif
    }

    // 地表上摆放的物件，只有在编辑器下，才这样创建
    // 在游戏模式下，地表上的物件，将是动态加载的，并且父节点是 TLSeamlessMap
#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 )
    if( CCEGLView::sharedOpenGLView()->getIsEditorMode() )
    {
        for( int i=0; i < mbData.models_size(); ++i )
        {
            const framework::ModelInfo& mi = mbData.models( i );

            CCNode* pkModel = addModel( mi.model_file(), mi.x(), mi.y() );
            scaleObject( pkModel, mi.scale() );
            rotateObject( pkModel, mi.rotation() );
            setObjectOffset( pkModel, mi.offset_x(), mi.offset_y() );
        }

        setSelectedObject( NULL );
    }
#endif

    recreateGridLine();

    return true;
}

void TLMapBlock::clear()
{
    m_nRow = 0;
    m_nCol = 0;

#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
    std::list<SpriteInfo*>::iterator iter = m_listAllSprites.begin();
    std::list<SpriteInfo*>::iterator iter_end = m_listAllSprites.end();
    for( ; iter != iter_end; ++iter )
    {
        SpriteInfo* pSpriteInfo = (*iter);
        delete pSpriteInfo;
    }
    m_listAllSprites.clear();

    std::list<ModelInfo*>::iterator iter1 = m_listAllModels.begin();
    std::list<ModelInfo*>::iterator iter1_end = m_listAllModels.end();
    for( ; iter1 != iter1_end; ++iter1 )
    {
        ModelInfo* pkModelInfo = (*iter1);
        delete pkModelInfo;
    }
    m_listAllModels.clear();
#endif

    m_vecGridStates.clear();
}

void TLMapBlock::setIsEnablePassByIndex( int nIndex, bool bIsEnable )
{
    if( nIndex < 0 || nIndex >= (int)m_vecGridStates.size() )
        return;

    if( bIsEnable )
    {
        m_vecGridStates[nIndex] |= TL_GRID_FLAG_PASS;
    }
    else
    {
        m_vecGridStates[nIndex] &= (~TL_GRID_FLAG_PASS);
    }
}

bool TLMapBlock::getIsEnablePassByIndex( int nIndex ) const
{
    if( nIndex < 0 || nIndex >= (int)m_vecGridStates.size() )
        return false;

	return ( m_vecGridStates[nIndex] & TL_GRID_FLAG_PASS ) ? true : false;
}

void TLMapBlock::setIsEnablePass( float world_x, float world_y, bool bIsEnable )
{
    convertLocal( world_x, world_y );

    setIsEnablePassByIndex( getGridIndex( world_x, world_y ), bIsEnable );
}

bool TLMapBlock::getIsEnablePass( float world_x, float world_y )
{
    convertLocal( world_x, world_y );

    return getIsEnablePassByIndex( getGridIndex( world_x, world_y ) );
}

void TLMapBlock::setIsEnablePlantByIndex( int nIndex, bool bIsEnable )
{
    if( nIndex < 0 || nIndex >= (int)m_vecGridStates.size() )
        return;

    if( bIsEnable )
    {
        m_vecGridStates[nIndex] |= TL_GRID_FLAG_PLANT;
    }
    else
    {
        m_vecGridStates[nIndex] &= (~TL_GRID_FLAG_PLANT);
    }
}

bool TLMapBlock::getIsEnablePlantByIndex( int nIndex ) const
{
    if( nIndex < 0 || nIndex >= (int)m_vecGridStates.size() )
        return false;

	return ( m_vecGridStates[nIndex] & TL_GRID_FLAG_PLANT ) ? true : false;
}

void TLMapBlock::setIsEnablePlant( float world_x, float world_y, bool bIsEnable )
{
    convertLocal( world_x, world_y );

    setIsEnablePlantByIndex( getGridIndex( world_x, world_y ), bIsEnable );
}

bool TLMapBlock::getIsEnablePlant( float world_x, float world_y )
{
    convertLocal( world_x, world_y );

    return getIsEnablePlantByIndex( getGridIndex( world_x, world_y ) );
}

void TLMapBlock::setIsEnableFillWaterByIndex( int nIndex, bool bIsEnable )
{
    if( nIndex < 0 || nIndex >= (int)m_vecGridStates.size() )
        return;

    if( bIsEnable )
    {
        m_vecGridStates[nIndex] |= TL_GRID_FLAG_FILL_WATER;
    }
    else
    {
        m_vecGridStates[nIndex] &= (~TL_GRID_FLAG_FILL_WATER);
    }
}

bool TLMapBlock::getIsEnableFillWaterByIndex( int nIndex ) const
{
    if( nIndex < 0 || nIndex >= (int)m_vecGridStates.size() )
        return false;

	return ( m_vecGridStates[nIndex] & TL_GRID_FLAG_FILL_WATER ) ? true : false;
}

void TLMapBlock::setIsEnableFillWater( float world_x, float world_y, bool bIsEnable )
{
    convertLocal( world_x, world_y );

    setIsEnablePlantByIndex( getGridIndex( world_x, world_y ), bIsEnable );
}

bool TLMapBlock::getIsEnableFillWater( float world_x, float world_y )
{
    convertLocal( world_x, world_y );

    return getIsEnablePlantByIndex( getGridIndex( world_x, world_y ) );
}

void TLMapBlock::convertLocal( float& x, float& y )
{
}

int TLMapBlock::getGridIndex( float x, float y )
{
    int nBlockWidth = m_nWidth * m_nCol;
    int nBlockHeight = m_nHeight * m_nRow;

    int nCol = (int)( ( nBlockWidth * 0.5 + x ) / m_nWidth );
    int nRow = (int)( ( nBlockHeight * 0.5 - y ) / m_nHeight );

    return nRow * m_nCol + nCol;
}

void TLMapBlock::updateMaterial( const std::string& strMaterial, int nBlockWidth, int nBlockHeight )
{
    if( m_pMaterialBatchNode != NULL )
    {
        m_pMaterialBatchNode->removeFromParentAndCleanup( true );
        m_pMaterialBatchNode = NULL;
    }

    // 地表材质
	CCTexture2D* pMaterialTexture = MCLoader::sharedMCLoader()->loadTexture( strMaterial.c_str() );
    if( pMaterialTexture == NULL )
        return;

    m_pMaterialBatchNode = CCSpriteBatchNode::createWithTexture( pMaterialTexture );
    addChild( m_pMaterialBatchNode, -1000 );

    CCSprite* pFirstMaterialSprite = MCLoader::sharedMCLoader()->loadSprite( strMaterial.c_str() );
    m_pMaterialBatchNode->addChild( pFirstMaterialSprite );
    const CCSize& size = pFirstMaterialSprite->getContentSize();

    int nRowCount = (int)( nBlockHeight / size.height );
    if( nRowCount * size.height < nBlockHeight )
        nRowCount = nRowCount + 1;

    int nColCount = (int)( nBlockWidth / size.width );
    if( nColCount * size.width < nBlockWidth )
        nColCount = nColCount + 1;

    int nRealWidth = (int)( nColCount * size.width );
    int nRealHeight = (int)( nRowCount * size.height );

    float x = ( size.width - nRealWidth ) * 0.5f;
    float y = ( nRealHeight - size.height ) * 0.5f;
    pFirstMaterialSprite->setPosition( CCPoint( x, y ) );

    for( int i=0; i < nRowCount; ++i )
    {
        float y = ( nRealHeight - size.height ) * 0.5f - i * size.height;
        for( int j=0; j < nColCount; ++j )
        {
            if( i==0 && j== 0 )
                continue;

            CCSprite* pMaterialSprite = MCLoader::sharedMCLoader()->loadSprite( strMaterial.c_str() );
            m_pMaterialBatchNode->addChild( pMaterialSprite );

            float x = ( size.width - nRealWidth ) * 0.5f + j * size.width;
            pMaterialSprite->setPosition( CCPoint( x, y ) );
        }
    }
}

#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
void TLMapBlock::save()
{
    FILE* fp = fopen( m_strMapBlockFile.c_str(), "wb" );
    if( fp != NULL )
    {
        framework::MapBlock mbData;
		mbData.set_name( m_strBlockName );
        mbData.set_row( m_nRow );
        mbData.set_col( m_nCol );
        mbData.set_width( m_nWidth );
        mbData.set_height( m_nHeight );
		mbData.set_material( m_strMaterial );

        for( int i=0; i < (int)m_vecGridStates.size(); ++i )
            mbData.add_grid_states( m_vecGridStates[i] );

        std::list<SpriteInfo*>::iterator iter = m_listAllSprites.begin();
        std::list<SpriteInfo*>::iterator iter_end = m_listAllSprites.end();
        for( ; iter != iter_end; ++iter )
        {
            SpriteInfo* pSpriteInfo = (*iter);

            framework::SpriteInfo* si = mbData.add_sprites();
            si->set_file( pSpriteInfo->strFileName );
            si->set_x( pSpriteInfo->x );
            si->set_y( pSpriteInfo->y );
            si->set_scale( pSpriteInfo->scale );
            si->set_rotation( pSpriteInfo->rotation );
            si->set_z_order( pSpriteInfo->z_order );
        }

        std::list<ModelInfo*>::iterator iter1 = m_listAllModels.begin();
        std::list<ModelInfo*>::iterator iter1_end = m_listAllModels.end();
        for( ; iter1 != iter1_end; ++iter1 )
        {
            ModelInfo* pkModelInfo = (*iter1);

            framework::ModelInfo* mi = mbData.add_models();
            mi->set_model_file( pkModelInfo->strFileName );
            mi->set_x( pkModelInfo->x );
            mi->set_y( pkModelInfo->y );
            mi->set_offset_x( pkModelInfo->offset_x );
            mi->set_offset_y( pkModelInfo->offset_y );
            mi->set_scale( pkModelInfo->scale );
            mi->set_rotation( pkModelInfo->rotation );
        }

        std::string strBuffer;
        mbData.SerializeToString( &strBuffer );

        fwrite( strBuffer.c_str(), strBuffer.length(), 1, fp );

        fclose( fp );
    }
}

void TLMapBlock::setMaterial( const std::string& strMaterial )
{
    updateMaterial( strMaterial, m_nCol * m_nWidth, m_nRow * m_nHeight );
}

CCNode* TLMapBlock::addSprite( const std::string& strFileName, float x, float y )
{
    CCSprite* pRetSprite = MCLoader::sharedMCLoader()->loadSprite( strFileName.c_str() );
	if( pRetSprite != NULL )
	{
		SpriteInfo* si = new SpriteInfo;
		si->pSprite = pRetSprite;
		si->strFileName = strFileName;
		si->x = x;
		si->y = y;
		si->scale = 1.0f;
		si->rotation = 0.0f;
        si->z_order = 0;

		m_listAllSprites.push_back( si );

		pRetSprite->setPosition( CCPoint( x, y ) );
		addChild( pRetSprite );

		setSelectedObject( pRetSprite );
	}

	return pRetSprite;
}

CCNode* TLMapBlock::hitSprite( float x, float y )
{
    int nZOrder = -9999999;
    CCNode* pRetSprite = NULL;

    std::list<SpriteInfo*>::iterator iter = m_listAllSprites.begin();
    std::list<SpriteInfo*>::iterator iter_end = m_listAllSprites.end();
    for( ; iter != iter_end; ++iter )
    {
        SpriteInfo* pSpriteInfo = (*iter);
        const CCPoint& position = pSpriteInfo->pSprite->getPosition();
        const CCSize& size = pSpriteInfo->pSprite->getContentSize();
        if( x >= position.x - size.width * 0.5f &&
            x <= position.x + size.width * 0.5f &&
            y >= position.y - size.height * 0.5f &&
            y <= position.y + size.height * 0.5f )
        {
            int z_order = pSpriteInfo->pSprite->getZOrder();
            if( z_order > nZOrder )
            {
                nZOrder = z_order;
                pRetSprite = pSpriteInfo->pSprite;
            }
        }
    }

	setSelectedObject( pRetSprite );

    return pRetSprite;
}

// model
CCNode* TLMapBlock::addModel( const std::string& strFileName, float x, float y )
{
    TLModel* pkRetModel = TLModel::createWithName( strFileName.c_str() );
	if( pkRetModel != NULL )
	{
		ModelInfo* mi = new ModelInfo;
		mi->pkModelNode = pkRetModel;
		mi->strFileName = strFileName;
		mi->x = x;
		mi->y = y;
        mi->offset_x = 0;
        mi->offset_y = 0;
		mi->scale = 1.0f;
		mi->rotation = 0.0f;

		m_listAllModels.push_back( mi );

		pkRetModel->setPosition( CCPoint( x, y ) );
		addChild( pkRetModel, (int)y );

		setSelectedObject( pkRetModel );
	}

	return pkRetModel;
}

void TLMapBlock::removeObject( CCNode* pkNode )
{
    std::list<ModelInfo*>::iterator iter1 = m_listAllModels.begin();
    std::list<ModelInfo*>::iterator iter1_end = m_listAllModels.end();
    for( ; iter1 != iter1_end; ++iter1 )
    {
        ModelInfo* pkModelInfo = (*iter1);
        if( pkModelInfo->pkModelNode == pkNode )
        {
            pkNode->removeFromParentAndCleanup( true );

            m_listAllModels.erase( iter1 );

			setSelectedObject( NULL );

            return;
        }
    }

	std::list<SpriteInfo*>::iterator iter2 = m_listAllSprites.begin();
    std::list<SpriteInfo*>::iterator iter2_end = m_listAllSprites.end();
    for( ; iter2 != iter2_end; ++iter2 )
    {
        SpriteInfo* pSpriteInfo = (*iter2);
        if( pSpriteInfo->pSprite == pkNode )
        {
            pkNode->removeFromParentAndCleanup( true );

            m_listAllSprites.erase( iter2 );

			setSelectedObject( NULL );

            return;
        }
    }
}

void TLMapBlock::moveObject( CCNode* pkNode, float mv_x, float mv_y )
{
    std::list<ModelInfo*>::iterator iter1 = m_listAllModels.begin();
    std::list<ModelInfo*>::iterator iter1_end = m_listAllModels.end();
    for( ; iter1 != iter1_end; ++iter1 )
    {
        ModelInfo* pkModelInfo = (*iter1);
        if( pkModelInfo->pkModelNode == pkNode )
        {
            pkModelInfo->x = pkModelInfo->x + mv_x;
            pkModelInfo->y = pkModelInfo->y + mv_y;
            pkNode->setPosition( CCPoint( pkModelInfo->x + pkModelInfo->offset_x, pkModelInfo->y + pkModelInfo->offset_y ) );
            reorderChild( pkNode, (int)pkModelInfo->y );

			m_pSelectedNode->setPositionX( pkNode->getPositionX() );
			m_pSelectedNode->setPositionY( pkNode->getPositionY() );

            return;
        }
    }

	std::list<SpriteInfo*>::iterator iter2 = m_listAllSprites.begin();
    std::list<SpriteInfo*>::iterator iter2_end = m_listAllSprites.end();
    for( ; iter2 != iter2_end; ++iter2 )
    {
        SpriteInfo* pSpriteInfo = (*iter2);
        if( pSpriteInfo->pSprite == pkNode )
        {
            pSpriteInfo->x = pSpriteInfo->x + mv_x;
            pSpriteInfo->y = pSpriteInfo->y + mv_y;
            pkNode->setPosition( CCPoint( pSpriteInfo->x, pSpriteInfo->y ) );

			m_pSelMarkSprite->setPositionX( pkNode->getPositionX() );
			m_pSelMarkSprite->setPositionY( pkNode->getPositionY() );

            return;
        }
    }
}

void TLMapBlock::scaleObject( CCNode* pkNode, float scale )
{
    std::list<ModelInfo*>::iterator iter1 = m_listAllModels.begin();
    std::list<ModelInfo*>::iterator iter1_end = m_listAllModels.end();
    for( ; iter1 != iter1_end; ++iter1 )
    {
        ModelInfo* pkModelInfo = (*iter1);
        if( pkModelInfo->pkModelNode == pkNode )
        {
            pkModelInfo->scale *= scale;
            pkNode->setScale( pkModelInfo->scale );

            return;
        }
    }

	std::list<SpriteInfo*>::iterator iter2 = m_listAllSprites.begin();
    std::list<SpriteInfo*>::iterator iter2_end = m_listAllSprites.end();
    for( ; iter2 != iter2_end; ++iter2 )
    {
        SpriteInfo* pSpriteInfo = (*iter2);
        if( pSpriteInfo->pSprite == pkNode )
        {
            pSpriteInfo->scale *= scale;
            pkNode->setScale( pSpriteInfo->scale );

            return;
        }
    }
}

void TLMapBlock::rotateObject( CCNode* pkNode, float rotation )
{
    std::list<ModelInfo*>::iterator iter1 = m_listAllModels.begin();
    std::list<ModelInfo*>::iterator iter1_end = m_listAllModels.end();
    for( ; iter1 != iter1_end; ++iter1 )
    {
        ModelInfo* pkModelInfo = (*iter1);
        if( pkModelInfo->pkModelNode == pkNode )
        {
            pkModelInfo->rotation = rotation;
            pkNode->setRotation( pkModelInfo->rotation );

            return;
        }
    }

	std::list<SpriteInfo*>::iterator iter2 = m_listAllSprites.begin();
    std::list<SpriteInfo*>::iterator iter2_end = m_listAllSprites.end();
    for( ; iter2 != iter2_end; ++iter2 )
    {
        SpriteInfo* pSpriteInfo = (*iter2);
        if( pSpriteInfo->pSprite == pkNode )
        {
            pSpriteInfo->rotation = rotation;
            pkNode->setRotation( pSpriteInfo->rotation );

            return;
        }
    }
}

void TLMapBlock::setObjectOffset( CCNode* pkNode, float offset_x, float offset_y )
{
    std::list<ModelInfo*>::iterator iter = m_listAllModels.begin();
    std::list<ModelInfo*>::iterator iter_end = m_listAllModels.end();
    for( ; iter != iter_end; ++iter )
    {
        ModelInfo* pkModelInfo = (*iter);
        if( pkModelInfo->pkModelNode == pkNode )
        {
            pkModelInfo->offset_x = offset_x;
            pkModelInfo->offset_y = offset_y;
            pkNode->setPosition( CCPoint( pkModelInfo->x + pkModelInfo->offset_x, pkModelInfo->y + pkModelInfo->offset_y ) );

			m_pSelectedNode->setPositionX( pkNode->getPositionX() );
			m_pSelectedNode->setPositionY( pkNode->getPositionY() );

            return;
        }
    }
}

CCNode* TLMapBlock::hitModel( float x, float y )
{
    int nZOrder = -9999999;
    CCNode* pkRetModel = NULL;

    std::list<ModelInfo*>::iterator iter = m_listAllModels.begin();
    std::list<ModelInfo*>::iterator iter_end = m_listAllModels.end();
    for( ; iter != iter_end; ++iter )
    {
        ModelInfo* pkModelInfo = (*iter);
        const CCPoint& position = pkModelInfo->pkModelNode->getPosition();
        const CCSize& size = ((TLModel*)pkModelInfo->pkModelNode)->mcBoundingBox.size;
        if( x >= position.x + pkModelInfo->offset_x - size.width * 0.5f &&
            x <= position.x + pkModelInfo->offset_x + size.width * 0.5f &&
            y >= position.y + pkModelInfo->offset_y - size.height * 0.5f &&
            y <= position.y + pkModelInfo->offset_y + size.height * 0.5f )
        {
            int z_order = pkModelInfo->pkModelNode->getZOrder();
            if( z_order > nZOrder )
            {
                nZOrder = z_order;
                pkRetModel = pkModelInfo->pkModelNode;
            }
        }
    }

	setSelectedObject( pkRetModel );

    return pkRetModel;
}

void TLMapBlock::setSelectedObject( CCNode* pkSelObj )
{
	m_pSelectedNode = pkSelObj;
	m_pSelMarkSprite->setVisible( m_pSelectedNode ? true : false );

	if( m_pSelectedNode != NULL )
	{
		m_pSelMarkSprite->setPositionX( m_pSelectedNode->getPositionX() );
		m_pSelMarkSprite->setPositionY( m_pSelectedNode->getPositionY() );
	}
}

void TLMapBlock::create( int nRow, int nCol, int nWidth, int nHeight )
{
    // 先清理
    clear();

    // 
    m_nRow = nRow;
    m_nCol = nCol;
    m_nWidth = nWidth;
    m_nHeight = nHeight;

    int nGridState = TL_GRID_FLAG_PLANT | TL_GRID_FLAG_PASS;
    m_vecGridStates.resize( nRow * nCol, nGridState );

    // 重新生成网格
    recreateGridLine();
}

#endif
void TLMapBlock::recreateGridLine()
{
    float fHalfTotalWidth = m_nCol * m_nWidth * 0.5f;
    float fHalfTotalHeight = m_nRow * m_nHeight * 0.5f;

    m_pGridBatchNode->removeAllChildrenWithCleanup( true );

    for( int i=0; i < m_nRow; ++i )
    {
        for( int j=0; j < m_nCol; ++j )
        {
            CCSprite* pSprite = MCLoader::sharedMCLoader()->loadSprite( GRID_SPRITE_FILE );
            //const CCSize& size = pSprite->getContentSize();

            float x = ( ( j + 0.5f ) * m_nWidth ) - fHalfTotalWidth;
            float y = fHalfTotalHeight - ( ( i + 0.5f ) * m_nHeight );
            pSprite->setPosition( CCPoint( x, y ) );

            m_pGridBatchNode->addChild( pSprite );
        }
    }
}

void TLMapBlock::setIsShowGirdLine( bool bIsShow )
{
    m_bShowGridLine = bIsShow;
    m_pGridBatchNode->setVisible( bIsShow );
}
