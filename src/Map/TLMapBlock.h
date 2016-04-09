#pragma once
#include <sprite_nodes/CCSprite.h>
#include <vector>

// grid state flag ------------------------------------------------------------------------------------
enum TL_GRID_FLAG
{
	TL_GRID_FLAG_PASS				    = 0x00000001,					// 能否通行
	TL_GRID_FLAG_PLANT				    = 0x00000002,					// 能否种植
    TL_GRID_FLAG_FILL_WATER             = 0x00000004,                   // 能否取水
};

USING_NS_CC;

class TLMapBlock : public CCNode
{
public:
	TLMapBlock( const std::string& strMapBlockFile );
	virtual ~TLMapBlock();

    static TLMapBlock* create( const std::string& strFileName );

    virtual bool init();

    void clear();

public:
    void setIsEnablePassByIndex( int nIndex, bool bIsEnable );
    bool getIsEnablePassByIndex( int nIndex ) const;
    void setIsEnablePass( float world_x, float world_y, bool bIsEnable );
    bool getIsEnablePass( float world_x, float world_y );

    void setIsEnablePlantByIndex( int nIndex, bool bIsEnable );
    bool getIsEnablePlantByIndex( int nIndex ) const;
    void setIsEnablePlant( float world_x, float world_y, bool bIsEnable );
    bool getIsEnablePlant( float world_x, float world_y );

    void setIsEnableFillWaterByIndex( int nIndex, bool bIsEnable );
    bool getIsEnableFillWaterByIndex( int nIndex ) const;
    void setIsEnableFillWater( float world_x, float world_y, bool bIsEnable );
    bool getIsEnableFillWater( float world_x, float world_y );

    void convertLocal( float& x, float& y );
    int getGridIndex( float x, float y );

protected:
    std::string m_strMapBlockFile;
    CCSpriteBatchNode* m_pMaterialBatchNode;

    int m_nRow, m_nCol;
    int m_nWidth, m_nHeight;

    std::vector<int> m_vecGridStates;

    void updateMaterial( const std::string& strMaterial, int nBlockWidth, int nBlockHeight );

    // 下面的是编辑器独有的
#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
public:
    static bool newMapBlock( const std::string& strMapBlockFile, const std::string& strBlockName, int nRow, int nCol, int nGridWidth, int nGridHeight, const std::string& strMaterial );

    void create( int nRow, int nCol, int nWidth, int nHeight );
    void save();

    void setMaterial( const std::string& strMaterial );

    // sprite
	CCNode* addSprite( const std::string& strFileName, float x, float y );
    // model
	CCNode* addModel( const std::string& strFileName, float x, float y );

	void removeObject( CCNode* pkNode );
	void moveObject( CCNode* pkNode, float mv_x, float mv_y );
	void scaleObject( CCNode* pkNode, float scale );
	void rotateObject( CCNode* pkNode, float rotation );
	void setObjectOffset( CCNode* pkNode, float offset_x, float offset_y );

    CCNode* hitSprite( float x, float y );
    CCNode* hitModel( float x, float y );

	void setSelectedObject( CCNode* pkSelObj );

    //void recreateGridLine();
    //void setIsShowGirdLine( bool bIsShow );
    //bool getIsShowGridLine() const { return m_bShowGridLine; }

protected:
	std::string m_strBlockName;
    std::string m_strMaterial;

    struct SpriteInfo {
        CCNode*         pSprite;
        std::string     strFileName;
        float           x, y;
        float           scale;
        float           rotation;
        int             z_order;
    };
    std::list<SpriteInfo*> m_listAllSprites;

    struct ModelInfo {
        CCNode*         pkModelNode;
        std::string     strFileName;
        float           x, y;
        float           offset_x, offset_y;
        float           scale;
        float           rotation;
    };
    std::list<ModelInfo*> m_listAllModels;

	CCNode* m_pSelectedNode;
	CCSize m_kSelMarkSize;
	CCSprite* m_pSelMarkSprite;

#endif
    void recreateGridLine();
    void setIsShowGirdLine( bool bIsShow );
    bool getIsShowGridLine() const { return m_bShowGridLine; }

    bool m_bShowGridLine;
    CCSpriteBatchNode* m_pGridBatchNode;
};
